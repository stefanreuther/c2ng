/**
  *  \file interpreter/vmio/objectloader.cpp
  */

#include "interpreter/vmio/objectloader.hpp"
#include "afl/except/fileformatexception.hpp"
#include "util/translation.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/bits/value.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/io/limitedstream.hpp"
#include "interpreter/vmio/structures.hpp"
#include "interpreter/vmio/valueloader.hpp"
#include "interpreter/vmio/processloadcontext.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"

namespace {
    using interpreter::vmio::structures::UInt32_t;
    using interpreter::vmio::structures::Tag;

    String_t loadString(afl::io::Stream& s)
    {
        String_t result;
        uint8_t buffer[128];
        while (size_t n = s.read(buffer)) {
            result.append(static_cast<char*>(static_cast<void*>(buffer)), n);
        }
        return result;
    }

    // /** Load context list from stream.
    //     \param out      [in] Stream to read from
    //     \param slots    [in] Number of contexts to load
    //     \param contexts [out] Contexts will be added here
    //     \throws FileFormatException if an invalid context is encountered
 
    //     This is a very stripped-down version of IntDataSegment::load,
    //     which assumes to find only contexts, no scalars or other values. */
    void loadContexts(interpreter::Process& proc,
                      interpreter::vmio::LoadContext& ctx,
                      afl::io::Stream& in,
                      uint32_t count)
    {
        // ex int/contextio.h:loadContexts
        afl::base::GrowableMemory<Tag> headers;
        headers.resize(count);
        in.fullRead(headers.toBytes());

        // Read elements
        afl::base::Memory<const Tag> reader(headers);
        while (const Tag* p = reader.eat()) {
            interpreter::TagNode node;
            node.tag = p->packedTag;
            node.value = p->packedValue;
            interpreter::Context* cv = ctx.loadContext(node, in);
            if (!cv) {
                throw afl::except::FileFormatException(in, _("Invalid value in context list; file probably written by newer version of PCC"));
            }
            proc.pushNewContext(cv);
        }
    }

    class ArrayLoader {
     public:
        virtual void add(uint32_t value) = 0;
        void load(afl::io::Stream& in, uint32_t n)
            {
                // ex int/vmio.cc:loadArray32
                const size_t CHUNK = 128;
                while (n > 0) {
                    // Read into buffer
                    UInt32_t buffer[CHUNK];
                    afl::base::Memory<UInt32_t> now(buffer);
                    now.trim(n);
                    in.fullRead(now.toBytes());

                    // Process
                    while (UInt32_t* p = now.eat()) {
                        add(*p);
                        --n;
                    }
                }
            }
    };

    class CodeLoader : public ArrayLoader {
     public:
        CodeLoader(interpreter::BCORef_t bco)
            : m_bco(bco)
            { }
        virtual void add(uint32_t value)
            { m_bco->addInstruction(interpreter::Opcode::Major((value >> 24) & 255), uint8_t((value >> 16) & 255), uint16_t(value & 65535)); }
     private:
        interpreter::BCORef_t m_bco;
    };

    class LineLoader : public ArrayLoader {
     public:
        LineLoader(interpreter::BCORef_t bco)
            : m_bco(bco),
              m_hasAddress(false),
              m_address()
            { }
        virtual void add(uint32_t value)
            {
                if (m_hasAddress) {
                    m_bco->addLineNumber(value, m_address);
                    m_hasAddress = false;
                } else {
                    m_address = value;
                    m_hasAddress = true;
                }
            }
     private:
        interpreter::BCORef_t m_bco;
        bool m_hasAddress;
        uint32_t m_address;
    };

    class DimLoader : public ArrayLoader {
     public:
        DimLoader(interpreter::ArrayData& data, afl::io::Stream& in)
            : m_data(data),
              m_stream(in)
            { }
        virtual void add(uint32_t value)
            {
                if (!m_data.addDimension(value)) {
                    throw afl::except::FileProblemException(m_stream, _("Invalid array"));
                }
            }
     private:
        interpreter::ArrayData& m_data;
        afl::io::Stream& m_stream;
    };
}


class interpreter::vmio::ObjectLoader::LoadObject {
 public:
    LoadObject(afl::io::Stream& s)
        : m_stream(s),
          m_objectSize(0),
          m_propertyStream(),
          m_nextProperty(0),
          m_propertyId(0),
          m_nextObject(s.getPos()),
          m_properties()
        { }
    bool readObject(uint32_t& type, uint32_t& id);
    afl::io::Stream* readProperty(uint32_t& id, uint32_t& count);

    afl::io::Stream& getStream()
        { return m_stream; }

 private:
    afl::io::Stream& m_stream;
    uint32_t m_objectSize;
    std::auto_ptr<afl::io::Stream> m_propertyStream;
    afl::io::Stream::FileSize_t m_nextProperty;
    uint32_t m_propertyId;
    afl::io::Stream::FileSize_t m_nextObject;

    afl::base::GrowableMemory<UInt32_t> m_properties;

    void consumeObjectSize(uint32_t needed);
};

void
interpreter::vmio::ObjectLoader::LoadObject::consumeObjectSize(uint32_t needed)
{
    // ex ObjectLoader::checkObjSize
    if (needed > m_objectSize) {
        throw afl::except::FileFormatException(m_stream, _("Invalid size"));
    }
    m_objectSize -= needed;
}

bool
interpreter::vmio::ObjectLoader::LoadObject::readObject(uint32_t& type, uint32_t& id)
{
    // Read header
    structures::ObjectHeader header;

    m_stream.setPos(m_nextObject);
    const size_t n = m_stream.read(afl::base::fromObject(header));
    if (n == 0) {
        return false;
    }
    if (n != sizeof(header)) {
        throw afl::except::FileTooShortException(m_stream);
    }

    const uint32_t objectType  = header.type;
    const uint32_t objectId    = header.id;
    m_objectSize  = header.size;
    const uint32_t numProperties = header.numProperties;
    m_nextObject += sizeof(header) + m_objectSize;

    /* Validate */
    consumeObjectSize(numProperties * 8);

    // Read property headers
    m_properties.resize(2 * numProperties);
    m_stream.fullRead(m_properties.toBytes());

    // Initialize properties and skip first one
    m_nextProperty = m_stream.getPos();
    m_propertyId = 0;

    uint32_t tmp;
    readProperty(tmp, tmp);

    type = objectType;
    id = objectId;
    return true;
}

afl::io::Stream*
interpreter::vmio::ObjectLoader::LoadObject::readProperty(uint32_t& id, uint32_t& count)
{
    // Do we have another property?
    UInt32_t* pCount = m_properties.at(2 * m_propertyId);
    UInt32_t* pSize  = m_properties.at(2 * m_propertyId + 1);
    if (!pCount || !pSize) {
        return 0;
    }

    // Check property
    uint32_t propertySize = *pSize;
    uint32_t propertyId = m_propertyId++;
    uint32_t propertyCount = *pCount;
    consumeObjectSize(propertySize);

    // Initialize content
    m_propertyStream.reset(new afl::io::LimitedStream(m_stream, m_nextProperty, propertySize));
    m_nextProperty += propertySize;

    // Produce result
    id = propertyId;
    count = propertyCount;
    return m_propertyStream.get();
}


interpreter::vmio::ObjectLoader::ObjectLoader(afl::charset::Charset& cs, LoadContext& ctx)
    : m_charset(cs),
      m_context(ctx)
{ }

interpreter::vmio::ObjectLoader::~ObjectLoader()
{ }

// /** Load VM file.
//     \param s Stream to read from
//     \param acceptProcesses true to accept saved processes (=VM file),
//     false to accept only bytecode (=object file) */
void
interpreter::vmio::ObjectLoader::load(afl::io::Stream& s)
{
    // ex IntVMLoadContext::load
    LoadObject ldr(s);
    uint32_t objType, objId;
    while (ldr.readObject(objType, objId)) {
        switch (objType) {
         case structures::otyp_Process:
            loadProcess(ldr);
            break;

         case structures::otyp_Bytecode:
            loadBCO(ldr, objId);
            break;

         case structures::otyp_DataArray:
            loadArray(ldr, objId);
            break;

         case structures::otyp_DataHash:
            loadHash(ldr, objId);
            break;

         case structures::otyp_DataStructValue:
            loadStructureValue(ldr, objId);
            break;

         case structures::otyp_DataStructType:
            loadStructureType(ldr, objId);
            break;

         default:
            throw afl::except::FileFormatException(s, _("Unexpected object"));
        }
    }
}

// /** Get bytecode object by Id
//     \param id Id to check
//     \return bytecode object */
interpreter::BCORef_t
interpreter::vmio::ObjectLoader::getBCO(uint32_t id)
{
    // ex IntVMLoadContext::getBCO
    SubroutineValue* sv = bco_map[id];
    if (sv == 0) {
        sv = bco_map.insertNew(id, new SubroutineValue(*new BytecodeObject()));
    }
    return sv->getBytecodeObject();
}

// /** Get hash object by Id
//     \param id Id to check
//     \return hash data */
afl::base::Ref<interpreter::HashData>
interpreter::vmio::ObjectLoader::getHash(uint32_t id)
{
    // ex IntVMLoadContext::getHash
    HashValue* hv = hash_map[id];
    if (hv == 0) {
        hv = hash_map.insertNew(id, new HashValue(*new HashData()));
    }
    return hv->getData();
}

// /** Get array object by Id
//     \param id Id to check
//     \return array data */
afl::base::Ref<interpreter::ArrayData>
interpreter::vmio::ObjectLoader::getArray(uint32_t id)
{
    // ex IntVMLoadContext::getArray
    ArrayValue* av = array_map[id];
    if (av == 0) {
        av = array_map.insertNew(id, new ArrayValue(*new ArrayData()));
    }
    return av->getData();
}

// /** Get structure object by Id
//     \param id Id to check
//     \return structure data */
afl::base::Ref<interpreter::StructureValueData>
interpreter::vmio::ObjectLoader::getStructureValue(uint32_t id)
{
    // ex IntVMLoadContext::getStructureValue
    StructureValue* sv = struct_value_map[id];
    if (sv == 0) {
        // Create structure with a dummy type. This guarantees that all structures
        // actually have a type, even if the VM file is broken and doesn't create one.
        sv = struct_value_map.insertNew(id, new StructureValue(*new StructureValueData(*new StructureTypeData())));
    }
    return sv->getValue();
}

// /** Get structure type object by Id
//     \param id Id to check
//     \return structure type data */
afl::base::Ref<interpreter::StructureTypeData>
interpreter::vmio::ObjectLoader::getStructureType(uint32_t id)
{
    // ex IntVMLoadContext::getStructureType
    StructureType* sv = struct_type_map[id];
    if (sv == 0) {
        sv = struct_type_map.insertNew(id, new StructureType(*new StructureTypeData()));
    }
    return sv->getType();
}

afl::data::Value*
interpreter::vmio::ObjectLoader::loadBCO(uint32_t id)
{
    return new SubroutineValue(getBCO(id));
}

afl::data::Value*
interpreter::vmio::ObjectLoader::loadArray(uint32_t id)
{
    return new ArrayValue(getArray(id));
}

afl::data::Value*
interpreter::vmio::ObjectLoader::loadHash(uint32_t id)
{
    return new HashValue(getHash(id));
}

afl::data::Value*
interpreter::vmio::ObjectLoader::loadStructureValue(uint32_t id)
{
    return new StructureValue(getStructureValue(id));
}

afl::data::Value*
interpreter::vmio::ObjectLoader::loadStructureType(uint32_t id)
{
    return new StructureType(getStructureType(id));
}

// /** Load process. The process will be created in runnable state.
//     \param ldr LoadObject that has just read the object header */
void
interpreter::vmio::ObjectLoader::loadProcess(LoadObject& ldr)
{
    // ex IntVMLoadContext::loadProcess
    // Create process
    Process* proc = createProcess();
    if (!proc) {
        throw afl::except::FileFormatException(ldr.getStream(), _("Unexpected object"));
    }

    // remove contexts created by Process' constructor
    while (!proc->getContexts().empty()) {
        proc->popContext();
    }

    ProcessLoadContext ctx(*this, *proc);

    uint16_t loaded_context_tos = 0;
    try {
        uint32_t propId, propCount;
        while (afl::io::Stream* ps = ldr.readProperty(propId, propCount)) {
            switch (propId) {
             case 1: {
                // header
                structures::ProcessHeader hdr;
                size_t n = ps->read(afl::base::fromObject(hdr));
                if (n >= 1) {
                    proc->setPriority(hdr.priority);
                }
                if (n >= 2) {
                    proc->setProcessKind(hdr.kind);
                }
                if (n >= 4) {
                    loaded_context_tos = hdr.contextTOS;
                }
                break;
             }

             case 2:
                // name (string)
                proc->setName(loadString(*ps));
                break;

             case 3:
                // frames (object array)
                loadFrames(*proc, ctx, *ps, propCount);
                break;

             case 4:
                // contexts (data segment)
                loadContexts(*proc, ctx, *ps, propCount);
                break;

             case 5:
                // exceptions (counts = number, size = 16xcount)
                for (uint32_t i = 0, end = propCount; i < end; ++i) {
                    UInt32_t frame[4];
                    ps->fullRead(afl::base::fromObject(frame));

                    proc->pushExceptionHandler(frame[3], frame[0], frame[1], frame[2]);
                }
                break;

             case 6:
                // value stack (data segment)
                ValueLoader(m_charset, ctx).load(proc->getValueStack(), *ps, 0, propCount);
                break;

             default:
                break;
            }
        }
    }
    catch (...) {
        /* If loading fails with an exception, make sure that the process will not run */
        // FIXME: we should probably not throw; just log and proceed.
        proc->setState(Process::Terminated);
        throw;
    }

    /* Since context-TOS cannot be out of range, we do not install it
       without checking, but we cannot check it before we have loaded
       the whole process. */
    if (!proc->setContextTOS(loaded_context_tos)) {
        // Loaded context TOS was out of range. PCC2 ignores this, and so do we.
    }
}

// /** Load stack frames.
//     \param proc  Process to load stack frames into
//     \param s     Stream to read from
//     \param count Number of stack frames to read */
void
interpreter::vmio::ObjectLoader::loadFrames(Process& proc, LoadContext& ctx, afl::io::Stream& s, uint32_t count)
{
    // ex IntVMLoadContext::loadFrames
    // FIXME: does this need to be a method?
    LoadObject ldr(s);
    while (count-- > 0) {
        // Read frame object
        uint32_t objType, objId;
        if (!ldr.readObject(objType, objId)) {
            throw afl::except::FileFormatException(s, _("Invalid frame"));
        }
        if (objType != structures::otyp_Frame) {
            throw afl::except::FileFormatException(s, _("Invalid frame type"));
        }

        // Read frame content
        Process::Frame* frame = 0;
        uint32_t propId, propCount;
        while (afl::io::Stream* ps = ldr.readProperty(propId, propCount)) {
            switch (propId) {
             case 1: {
                // header
                structures::FrameHeader frameHeader;
                afl::base::fromObject(frameHeader).fill(0);
                size_t n = ps->read(afl::base::fromObject(frameHeader));

                // bco_id is mandatory
                if (n < 4) {
                    throw afl::except::FileFormatException(s, _("Invalid frame"));
                }

                // Create the frame
                std::auto_ptr<afl::data::Value> bco(ctx.loadBCO(frameHeader.bcoRef));
                SubroutineValue* sv = dynamic_cast<SubroutineValue*>(bco.get());
                if (sv == 0) {
                    throw afl::except::FileFormatException(s, _("Invalid frame"));
                }
                frame = &proc.pushFrame(sv->getBytecodeObject(), (frameHeader.flags & frameHeader.WantResult) != 0);

                // Other values
                frame->pc = frameHeader.pc;
                frame->contextSP = frameHeader.contextSP;
                frame->exceptionSP = frameHeader.exceptionSP;
                break;
             }

             case 2:
                // local values (data segment)
                if (frame == 0) {
                    throw afl::except::FileFormatException(s, _("Invalid frame"));
                }
                ValueLoader(m_charset, ctx).load(frame->localValues, *ps, 0, propCount);
                break;

             case 3:
                // local names (name list)
                if (frame == 0) {
                    throw afl::except::FileFormatException(s, _("Invalid frame"));
                }
                ValueLoader(m_charset, ctx).loadNames(frame->localNames, *ps, propCount);
                break;
            }
        }
    }
}


interpreter::Context*
interpreter::vmio::ObjectLoader::loadContext(const TagNode& tag, afl::io::Stream& aux)
{
    // FIXME: anything we need to handle ourselves?
    return m_context.loadContext(tag, aux);
}

interpreter::Context*
interpreter::vmio::ObjectLoader::loadMutex(const String_t& name, const String_t& note, Process* owner)
{
    return m_context.loadMutex(name, note, owner);
}

interpreter::Process*
interpreter::vmio::ObjectLoader::createProcess()
{
    return m_context.createProcess();
}


// /** Load bytecode object.
//     \param ldr Object loader that has just read the object header
//     \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadBCO(LoadObject& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadBCO
    /* Note: when implementing the merge-loaded-BCO-with-existing-identical
       optimisation, we must know whether this is the first instance of this
       BCO (optimisation applicable), or whether there already was a forward
       reference. The simplest way would be to duplicate getBCO here. */
    BCORef_t obj = getBCO(id);
    uint32_t propId, propCount;
    while (afl::io::Stream* ps = ldr.readProperty(propId, propCount)) {
        switch (propId) {
         case 1: {
            // Header
            structures::BCOHeader header;
            size_t n = ps->read(afl::base::fromObject(header));
            if (n >= 2) {
                obj->setIsProcedure((header.flags & header.ProcedureFlag) != 0);
                obj->setIsVarargs((header.flags & header.VarargsFlag) != 0);
            }
            if (n >= 4) {
                obj->setMinArgs(header.minArgs);
            }
            if (n >= 6) {
                obj->setMaxArgs(header.maxArgs);
            }
            if (n >= 8) {
                obj->setNumLabels(header.numLabels);
            }
            break;
         }

         case 2:
            // "data" (literals for pushlit, data segment)
            ValueLoader(m_charset, *this).load(obj->getLiterals(), *ps, 0, propCount);
            break;

         case 3:
            // "names" (names for e.g. pushvar, name list)
            ValueLoader(m_charset, *this).loadNames(obj->getNames(), *ps, propCount);
            break;

         case 4:
            // "code" (count = number of instructions, size = 4x count). 32 bit per instruction.
            CodeLoader(obj).load(*ps, propCount);
            break;

         case 5:
            // "local_names" (predeclared locals, name list)
            ValueLoader(m_charset, *this).loadNames(obj->getLocalNames(), *ps, propCount);
            break;

         case 6:
            // "name" (name hint for loading, string)
            obj->setName(loadString(*ps));
            break;

         case 7:
            // "file name" (debug file name, string)
            obj->setFileName(loadString(*ps));
            break;

         case 8:
            // "line numbers" (count = number of lines, size = 8x count)
            LineLoader(obj).load(*ps, propCount*2);
            break;

         default:
            break;
        }
    }
}

// /** Load hash object.
//     \param ldr Object loader that has just read the object header
//     \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadHash(LoadObject& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadHash
    afl::base::Ref<HashData> hash = getHash(id);
    uint32_t propId, propCount;
    while (afl::io::Stream* ps = ldr.readProperty(propId, propCount)) {
        switch (propId) {
         case 1:
            // names
            ValueLoader(m_charset, *this).loadNames(hash->getNames(), *ps, propCount);
            break;

         case 2:
            // values
            ValueLoader(m_charset, *this).load(hash->getContent(), *ps, 0, propCount);
            break;

         default:
            break;
        }
    }
}

// /** Load array object.
//     \param ldr Object loader that has just read the object header
//     \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadArray(LoadObject& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadArray
    afl::base::Ref<ArrayData> array = getArray(id);
    uint32_t propId, propCount;
    while (afl::io::Stream* ps = ldr.readProperty(propId, propCount)) {
        switch (propId) {
         case 1:
            // Dimensions. Since these can be used to do evil things, we do not
            // read them directly into the object, but into a temporary buffer
            // where we validate them by using the public API.
            DimLoader(*array, *ps).load(*ps, propCount);
            break;

         case 2:
            // values
            ValueLoader(m_charset, *this).load(array->content, *ps, 0, propCount);
            break;

         default:
            break;
        }
    }
}

// /** Load structure value.
//     \param ldr Object loader that has just read the object header
//     \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadStructureValue(LoadObject& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadStructureValue
    afl::base::Ref<StructureValueData> value = getStructureValue(id);
    uint32_t propId, propCount;
    while (afl::io::Stream* ps = ldr.readProperty(propId, propCount)) {
        switch (propId) {
         case 1:
            // Header
            {
                // \change PCC2 would accept getting no header, but this makes no sense
                UInt32_t type;
                ps->fullRead(afl::base::fromObject(type));
                value->type.reset(*getStructureType(type));
            }
            break;
         case 2:
            // content
            ValueLoader(m_charset, *this).load(value->data, *ps, 0, propCount);
            break;

         default:
            break;
        }
    }
}

// /** Load structure type.
//     \param ldr Object loader that has just read the object header
//     \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadStructureType(LoadObject& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadStructureType
    afl::base::Ref<StructureTypeData> type = getStructureType(id);
    uint32_t propId, propCount;
    while (afl::io::Stream* ps = ldr.readProperty(propId, propCount)) {
        switch (propId) {
         case 1:
            // names
            ValueLoader(m_charset, *this).loadNames(type->names, *ps, propCount);
            break;

         default:
            break;
        }
    }
}
