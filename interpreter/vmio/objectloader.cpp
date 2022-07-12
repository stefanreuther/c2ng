/**
  *  \file interpreter/vmio/objectloader.cpp
  *  \brief Class interpreter::vmio::ObjectLoader
  */

#include "interpreter/vmio/objectloader.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/io/limitedstream.hpp"
#include "interpreter/vmio/processloadcontext.hpp"
#include "interpreter/vmio/structures.hpp"
#include "interpreter/vmio/valueloader.hpp"

namespace {
    using interpreter::vmio::structures::UInt32_t;
    using interpreter::vmio::structures::Tag;
    using afl::io::Stream;

    /* Load string, without character translation.
       Used for process name, BCO name, BCO file name. */
    String_t loadString(Stream& s)
    {
        String_t result;
        uint8_t buffer[128];
        while (size_t n = s.read(buffer)) {
            result.append(static_cast<char*>(static_cast<void*>(buffer)), n);
        }
        return result;
    }

    /* Load context list from stream.
       \param [out] proc   Process (receives contexts)
       \param [in]  ctx    LoadContext (provides context repertoire)
       \param [in]  tx     Translator (for error messages)
       \param [in]  in     Stream to read
       \param [in]  count  Number of contexts to read

       \throws afl::except::FileFormatException if an invalid context is encountered

       This is a simplified version of ValueLoader::load() that assumes to find only contexts, no scalars or other values. */
    void loadContexts(interpreter::Process& proc, interpreter::vmio::LoadContext& ctx, afl::string::Translator& tx, Stream& in, uint32_t count)
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
                throw afl::except::FileFormatException(in, tx("Invalid value in context list; file probably written by newer version of PCC"));
            }
            proc.pushNewContext(cv);
        }
    }

    /* Loading an array-of-32-bit-values property */
    class ArrayLoader {
     public:
        virtual ~ArrayLoader()
            { }
        virtual void add(uint32_t value) = 0;
        void load(Stream& in, uint32_t n);
    };

    /* Implementation of ArrayLoader to load bytecode */
    class CodeLoader : public ArrayLoader {
     public:
        CodeLoader(interpreter::BytecodeObject& bco)
            : m_bco(bco)
            { }
        virtual void add(uint32_t value)
            { m_bco.addInstruction(interpreter::Opcode::Major((value >> 24) & 255), uint8_t((value >> 16) & 255), uint16_t(value & 65535)); }
     private:
        interpreter::BytecodeObject& m_bco;
    };

    /* Implementation of ArrayLoader to load line-number information */
    class LineLoader : public ArrayLoader {
     public:
        LineLoader(interpreter::BytecodeObject& bco)
            : m_bco(bco),
              m_hasAddress(false),
              m_address()
            { }
        virtual void add(uint32_t value)
            {
                if (m_hasAddress) {
                    m_bco.addLineNumber(value, m_address);
                    m_hasAddress = false;
                } else {
                    m_address = value;
                    m_hasAddress = true;
                }
            }
     private:
        interpreter::BytecodeObject& m_bco;
        bool m_hasAddress;
        uint32_t m_address;
    };

    /* Implementation of ArrayLoader to load array dimensions */
    class DimLoader : public ArrayLoader {
     public:
        DimLoader(interpreter::ArrayData& data, Stream& in, afl::string::Translator& tx)
            : m_data(data),
              m_stream(in),
              m_translator(tx)
            { }
        virtual void add(uint32_t value)
            {
                if (!m_data.addDimension(value)) {
                    throw afl::except::FileProblemException(m_stream, m_translator("Invalid array"));
                }
            }
     private:
        interpreter::ArrayData& m_data;
        Stream& m_stream;
        afl::string::Translator& m_translator;
    };


    void ArrayLoader::load(Stream& in, uint32_t n)
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
}

/*
 *  ChunkLoader: Load an object consisting of property chunks
 */

class interpreter::vmio::ObjectLoader::ChunkLoader {
 public:
    ChunkLoader(Stream& s, afl::string::Translator& tx)
        : m_stream(s),
          m_translator(tx),
          m_objectSize(0),
          m_propertyStream(),
          m_nextProperty(0),
          m_propertyId(0),
          m_nextObject(s.getPos()),
          m_properties()
        { }

    /** Read an object.
        \param [out] type Object type
        \param [out] id   Object Id
        \return true on success; false on EOF */
    bool readObject(uint32_t& type, uint32_t& id);

    /** Read a property.
        \param [out] id    Property Id
        \param [out] count Number of elements (property-specific)
        \return stream (usable with afl::base::Ref) to read property content; null if no more properties */
    Stream* readProperty(uint32_t& id, uint32_t& count);

 private:
    Stream& m_stream;
    afl::string::Translator& m_translator;
    uint32_t m_objectSize;
    afl::base::Ptr<Stream> m_propertyStream;
    Stream::FileSize_t m_nextProperty;
    uint32_t m_propertyId;
    Stream::FileSize_t m_nextObject;
    afl::base::GrowableMemory<UInt32_t> m_properties;

    void consumeObjectSize(uint32_t needed);
};

void
interpreter::vmio::ObjectLoader::ChunkLoader::consumeObjectSize(uint32_t needed)
{
    // ex ObjectLoader::checkObjSize
    if (needed > m_objectSize) {
        throw afl::except::FileFormatException(m_stream, m_translator("Invalid size"));
    }
    m_objectSize -= needed;
}

bool
interpreter::vmio::ObjectLoader::ChunkLoader::readObject(uint32_t& type, uint32_t& id)
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

Stream*
interpreter::vmio::ObjectLoader::ChunkLoader::readProperty(uint32_t& id, uint32_t& count)
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
    m_propertyStream = new afl::io::LimitedStream(m_stream, m_nextProperty, propertySize);
    m_nextProperty += propertySize;

    // Produce result
    id = propertyId;
    count = propertyCount;
    return m_propertyStream.get();
}


/*
 *  ObjectLoader
 */

// Constructor.
interpreter::vmio::ObjectLoader::ObjectLoader(afl::charset::Charset& cs, afl::string::Translator& tx, LoadContext& ctx)
    : m_BCOsById(),
      m_HashById(),
      m_ArrayById(),
      m_StructureValueById(),
      m_StructureTypeById(),
      m_charset(cs),
      m_translator(tx),
      m_context(ctx)
{ }

// Destructor.
interpreter::vmio::ObjectLoader::~ObjectLoader()
{ }

// Load object (*.qc) file.
interpreter::BCORef_t
interpreter::vmio::ObjectLoader::loadObjectFile(afl::base::Ref<afl::io::Stream> s)
{
    // Read header
    structures::ObjectFileHeader header;
    s->fullRead(afl::base::fromObject(header));
    if (std::memcmp(header.magic, structures::OBJECT_FILE_MAGIC, sizeof(header.magic)) != 0
        || header.version != structures::OBJECT_FILE_VERSION
        || header.zero != 0
        || header.headerSize < structures::OBJECT_FILE_HEADER_SIZE)
    {
        throw afl::except::FileFormatException(*s, m_translator("Invalid file header"));
    }

    // Adjust file pointer
    s->setPos(s->getPos() + header.headerSize - structures::OBJECT_FILE_HEADER_SIZE);

    // Read
    load(s);

    // Produce result
    return getBCO(header.entry);
}

// Load virtual-machine file.
void
interpreter::vmio::ObjectLoader::load(afl::base::Ref<afl::io::Stream> s)
{
    // ex IntVMLoadContext::load
    // FIXME: the parameter must be a Ref<> because the stream will eventually end up in a LimitedStream
    // which requires a Ref<>. However, actually the LimitedStream should be fixed to not need a Ref<>.
    ChunkLoader ldr(*s, m_translator);
    uint32_t objType, objId;
    while (ldr.readObject(objType, objId)) {
        switch (objType) {
         case structures::otyp_Process:
            loadProcess(ldr, *s);
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
            throw afl::except::FileFormatException(*s, m_translator("Unexpected object"));
        }
    }
}

// Get bytecode object by Id.
interpreter::BCORef_t
interpreter::vmio::ObjectLoader::getBCO(uint32_t id)
{
    // ex IntVMLoadContext::getBCO
    SubroutineValue* sv = m_BCOsById[id];
    if (sv == 0) {
        sv = m_BCOsById.insertNew(id, new SubroutineValue(BytecodeObject::create(true)));
    }
    return sv->getBytecodeObject();
}

// Get hash object by Id.
afl::data::Hash::Ref_t
interpreter::vmio::ObjectLoader::getHash(uint32_t id)
{
    // ex IntVMLoadContext::getHash
    HashValue* hv = m_HashById[id];
    if (hv == 0) {
        hv = m_HashById.insertNew(id, new HashValue(afl::data::Hash::create()));
    }
    return hv->getData();
}

// Get array object by Id.
afl::base::Ref<interpreter::ArrayData>
interpreter::vmio::ObjectLoader::getArray(uint32_t id)
{
    // ex IntVMLoadContext::getArray
    ArrayValue* av = m_ArrayById[id];
    if (av == 0) {
        av = m_ArrayById.insertNew(id, new ArrayValue(*new ArrayData()));
    }
    return av->getData();
}

// Get structure value object by Id.
afl::base::Ref<interpreter::StructureValueData>
interpreter::vmio::ObjectLoader::getStructureValue(uint32_t id)
{
    // ex IntVMLoadContext::getStructureValue
    StructureValue* sv = m_StructureValueById[id];
    if (sv == 0) {
        // Create structure with a dummy type. This guarantees that all structures
        // actually have a type, even if the VM file is broken and doesn't create one.
        sv = m_StructureValueById.insertNew(id, new StructureValue(*new StructureValueData(*new StructureTypeData())));
    }
    return sv->getValue();
}

// Get structure type object by Id.
afl::base::Ref<interpreter::StructureTypeData>
interpreter::vmio::ObjectLoader::getStructureType(uint32_t id)
{
    // ex IntVMLoadContext::getStructureType
    StructureType* sv = m_StructureTypeById[id];
    if (sv == 0) {
        sv = m_StructureTypeById.insertNew(id, new StructureType(*new StructureTypeData()));
    }
    return sv->getType();
}

// LoadContext:
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

void
interpreter::vmio::ObjectLoader::finishProcess(Process& proc)
{
    m_context.finishProcess(proc);
}

/** Load bytecode object.
    \param ldr ChunkLoader that has just read the object header
    \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadBCO(ChunkLoader& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadBCO
    /* Note: when implementing the merge-loaded-BCO-with-existing-identical
       optimisation, we must know whether this is the first instance of this
       BCO (optimisation applicable), or whether there already was a forward
       reference. The simplest way would be to duplicate getBCO here. */
    BCORef_t obj = getBCO(id);
    uint32_t propId, propCount;
    while (Stream* ps = ldr.readProperty(propId, propCount)) {
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
            ValueLoader(m_charset, *this, m_translator).load(obj->literals(), *ps, 0, propCount);
            break;

         case 3:
            // "names" (names for e.g. pushvar, name list)
            ValueLoader(m_charset, *this, m_translator).loadNames(obj->names(), *ps, propCount);
            break;

         case 4:
            // "code" (count = number of instructions, size = 4x count). 32 bit per instruction.
            CodeLoader(*obj).load(*ps, propCount);
            break;

         case 5:
            // "local_names" (predeclared locals, name list)
            ValueLoader(m_charset, *this, m_translator).loadNames(obj->localVariables(), *ps, propCount);
            break;

         case 6:
            // "name" (name hint for loading, string)
            obj->setSubroutineName(loadString(*ps));
            break;

         case 7:
            // "file name" (debug file name, string)
            obj->setFileName(loadString(*ps));
            break;

         case 8:
            // "line numbers" (count = number of lines, size = 8x count)
            LineLoader(*obj).load(*ps, propCount*2);
            break;

         default:
            break;
        }
    }
}

/** Load hash object.
    \param ldr ChunkLoader that has just read the object header
    \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadHash(ChunkLoader& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadHash
    // Load
    afl::data::NameMap names;
    afl::data::Segment values;
    uint32_t propId, propCount;
    while (Stream* ps = ldr.readProperty(propId, propCount)) {
        switch (propId) {
         case 1:
            // names
            ValueLoader(m_charset, *this, m_translator).loadNames(names, *ps, propCount);
            break;

         case 2:
            // values
            ValueLoader(m_charset, *this, m_translator).load(values, *ps, 0, propCount);
            break;

         default:
            break;
        }
    }

    // Store in hash
    afl::data::Hash::Ref_t hash = getHash(id);
    for (size_t i = 0, n = names.getNumNames(); i < n; ++i) {
        hash->setNew(names.getNameByIndex(i), values.extractElement(i));
    }
}

/** Load array object.
    \param ldr ChunkLoader that has just read the object header
    \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadArray(ChunkLoader& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadArray
    afl::base::Ref<ArrayData> array = getArray(id);
    uint32_t propId, propCount;
    while (Stream* ps = ldr.readProperty(propId, propCount)) {
        switch (propId) {
         case 1:
            // Dimensions. Since these can be used to do evil things, we do not
            // read them directly into the object, but into a temporary buffer
            // where we validate them by using the public API.
            DimLoader(*array, *ps, m_translator).load(*ps, propCount);
            break;

         case 2:
            // values
            ValueLoader(m_charset, *this, m_translator).load(array->content, *ps, 0, propCount);
            break;

         default:
            break;
        }
    }
}

/** Load structure value.
    \param ldr ChunkLoader that has just read the object header
    \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadStructureValue(ChunkLoader& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadStructureValue
    afl::base::Ref<StructureValueData> value = getStructureValue(id);
    uint32_t propId, propCount;
    while (Stream* ps = ldr.readProperty(propId, propCount)) {
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
            ValueLoader(m_charset, *this, m_translator).load(value->data, *ps, 0, propCount);
            break;

         default:
            break;
        }
    }
}

/** Load structure type.
    \param ldr ChunkLoader that has just read the object header
    \param id Id of object */
void
interpreter::vmio::ObjectLoader::loadStructureType(ChunkLoader& ldr, uint32_t id)
{
    // ex IntVMLoadContext::loadStructureType
    afl::base::Ref<StructureTypeData> type = getStructureType(id);
    uint32_t propId, propCount;
    while (Stream* ps = ldr.readProperty(propId, propCount)) {
        switch (propId) {
         case 1:
            // names
            ValueLoader(m_charset, *this, m_translator).loadNames(type->names(), *ps, propCount);
            break;

         default:
            break;
        }
    }
}

/** Load process.
    The process will be created in runnable state on success.
    \param ldr ChunkLoader that has just read the object header
    \param outerStream Outer stream (for generating error messages) */
void
interpreter::vmio::ObjectLoader::loadProcess(ChunkLoader& ldr, afl::io::Stream& outerStream)
{
    // ex IntVMLoadContext::loadProcess
    // Create process
    Process* proc = createProcess();
    if (!proc) {
        throw afl::except::FileFormatException(outerStream, m_translator("Unexpected object"));
    }

    // remove contexts created by Process' constructor
    while (!proc->getContexts().empty()) {
        proc->popContext();
    }

    // ProcessLoadContext enables us to create FrameContext's.
    ProcessLoadContext ctx(*this, *proc);

    uint16_t loaded_context_tos = 0;
    try {
        uint32_t propId, propCount;
        while (Stream* ps = ldr.readProperty(propId, propCount)) {
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
                loadContexts(*proc, ctx, m_translator, *ps, propCount);
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
                ValueLoader(m_charset, ctx, m_translator).load(proc->getValueStack(), *ps, 0, propCount);
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

    /* Since context-TOS must not be out of range, we do not install it
       without checking, but we cannot check it before we have loaded
       the whole process. */
    if (!proc->setContextTOS(loaded_context_tos)) {
        // Loaded context TOS was out of range. PCC2 ignores this, and so do we.
    }

    // Finish the process (put it in its place according to priority)
    finishProcess(*proc);
}

/** Load stack frames.
    \param proc  Process to load stack frames into
    \param ctx   LoadContext
    \param s     Stream to read from
    \param count Number of stack frames to read */
void
interpreter::vmio::ObjectLoader::loadFrames(Process& proc, LoadContext& ctx, afl::io::Stream& s, uint32_t count)
{
    // ex IntVMLoadContext::loadFrames
    // FIXME: does this need to be a method?
    ChunkLoader ldr(s, m_translator);
    while (count-- > 0) {
        // Read frame object
        uint32_t objType, objId;
        if (!ldr.readObject(objType, objId)) {
            throw afl::except::FileFormatException(s, m_translator("Invalid frame"));
        }
        if (objType != structures::otyp_Frame) {
            throw afl::except::FileFormatException(s, m_translator("Invalid frame type"));
        }

        // Read frame content
        Process::Frame* frame = 0;
        uint32_t propId, propCount;
        while (Stream* ps = ldr.readProperty(propId, propCount)) {
            switch (propId) {
             case 1: {
                // header
                structures::FrameHeader frameHeader;
                afl::base::fromObject(frameHeader).fill(0);
                size_t n = ps->read(afl::base::fromObject(frameHeader));

                // bco_id is mandatory
                if (n < 4) {
                    throw afl::except::FileFormatException(s, m_translator("Invalid frame"));
                }

                // Create the frame
                std::auto_ptr<afl::data::Value> bco(ctx.loadBCO(frameHeader.bcoRef));
                SubroutineValue* sv = dynamic_cast<SubroutineValue*>(bco.get());
                if (sv == 0) {
                    throw afl::except::FileFormatException(s, m_translator("Invalid frame"));
                }
                frame = &proc.pushFrame(sv->getBytecodeObject(), (frameHeader.flags & frameHeader.WantResult) != 0);

                // Other values
                frame->pc = frameHeader.pc;
                frame->contextSP = frameHeader.contextSP;
                frame->exceptionSP = frameHeader.exceptionSP;

                // Creating the frame will set up BCO locals.
                // We load these later.
                afl::data::NameMap().swap(frame->localNames);

                // Creating the frame will have created a FrameContext.
                // We will create the FrameContext later when loading contexts, so we do not need it.
                proc.popContext();
                break;
             }

             case 2:
                // local values (data segment)
                if (frame == 0) {
                    throw afl::except::FileFormatException(s, m_translator("Invalid frame"));
                }
                ValueLoader(m_charset, ctx, m_translator).load(frame->localValues, *ps, 0, propCount);
                break;

             case 3:
                // local names (name list)
                if (frame == 0) {
                    throw afl::except::FileFormatException(s, m_translator("Invalid frame"));
                }
                ValueLoader(m_charset, ctx, m_translator).loadNames(frame->localNames, *ps, propCount);
                break;
            }
        }
    }
}



