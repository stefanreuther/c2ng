/**
  *  \file interpreter/vmio/filesavecontext.cpp
  *  \brief Class interpreter::vmio::FileSaveContext
  *
  *  PCC2 Comment:
  *
  *  A VM file contains a pool of objects, namely
  *  - bytecode objects (BCOs, IntBytecodeObject)
  *  - processes (IntExecutionFrame)
  *  - data (IntHashData, IntArrayData)
  *
  *  Data is stored properly referenced and is not flattened. This is
  *  required to keep suspended call-by-reference intact, for example,
  *  code like this:
  *  <code>
  *     Sub foo(a)
  *       Stop
  *       a("x") := "y"
  *     EndSub
  *     Sub bar()
  *       Local a := NewHash()
  *       foo(a)
  *       Print a("x")
  *     EndSub
  *  </code>
  *  This implies we also have to deal with forward references.
  *
  *  Upon saving, we assign each object an Id. We also build a plan
  *  with a save order that minimizes forward references, by saving
  *  each object's preconditions before the object itself.
  *
  *  Upon loading, when we encounter an object reference without
  *  having seen the object yet, we create a blank object. When the
  *  object finally appears in the file, it is filled. It is an error
  *  for a VM file to contain references to objects it does not
  *  contain, but this error is not (yet) detected; it's harmless
  *  because empty objects are still valid.
  */

#include "interpreter/vmio/filesavecontext.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/bits/pack.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/tmp/ifthenelse.hpp"
#include "afl/tmp/issametype.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/savevisitor.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/vmio/processsavecontext.hpp"
#include "interpreter/vmio/structures.hpp"

namespace {
    using interpreter::vmio::structures::UInt32_t;

    class SaveObject {
     public:
        explicit SaveObject(afl::io::Stream& s);
        void start(uint32_t type, uint32_t id, uint32_t nprop);
        void end();
        void startProperty(uint32_t count);
        void endProperty();

     private:
        afl::io::Stream& m_stream;

        interpreter::vmio::structures::ObjectHeader m_header;
        afl::io::Stream::FileSize_t m_headerPosition;

        uint32_t m_propertyIndex;
        afl::io::Stream::FileSize_t m_thisPropertyPosition;
        afl::base::GrowableMemory<UInt32_t> m_properties;

        void writeHeader();
    };

    // Format a uint32_t.
    inline uint32_t toWord(uint32_t value)
    {
        // ex int/vmio.h:storeItem
        return value;
    }

    // Format a size_t.
    // The adventurous prototype is required because size_t may be the same as uint32_t and a size_t prototype cannot exist with the one above.
    // Therefore, if it is identical, we define the function taking a uint64_t instead (which is then never used).
    inline uint32_t toWord(afl::tmp::IfThenElse<afl::tmp::IsSameType<uint32_t, size_t>::result, uint64_t, size_t>::Type value)
    {
        // ex int/vmio.h:storeItem
        return static_cast<uint32_t>(value);
    }

    // Format a Opcode.
    inline uint32_t toWord(interpreter::Opcode insn)
    {
        // ex int/vmio.h:storeItem
        return uint32_t(insn.arg)
            + (uint32_t(insn.minor) << 16)
            + (uint32_t(insn.getExternalMajor()) << 24);
    }

    template<typename T>
    void
    writeArray32(afl::io::Stream& out, afl::base::Memory<const T> obj)
    {
        const size_t CHUNK = 128;
        while (!obj.empty()) {
            UInt32_t buffer[CHUNK];
            afl::base::Memory<const T> now(obj.split(CHUNK));
            size_t i = 0;
            while (const T* p = now.eat()) {
                buffer[i++] = toWord(*p);
            }
            out.fullWrite(afl::base::Memory<UInt32_t>(buffer).trim(i).toBytes());
        }
    }
}

SaveObject::SaveObject(afl::io::Stream& s)
    : m_stream(s),
      m_header(),
      m_headerPosition(0),
      m_propertyIndex(0),
      m_thisPropertyPosition(0),
      m_properties()
{ }

void
SaveObject::writeHeader()
{
    m_stream.fullWrite(afl::base::fromObject(m_header));
    m_stream.fullWrite(m_properties.toBytes());
}

void
SaveObject::start(uint32_t type, uint32_t id, uint32_t nprop)
{
    ++nprop;
    m_header.type = type;
    m_header.id = id;
    m_header.size = 0;
    m_header.numProperties = nprop;
    m_headerPosition = m_stream.getPos();
    m_properties.clear();
    m_properties.resize(2*nprop);
    m_properties.fill(UInt32_t());
    m_propertyIndex = 1;
    writeHeader();
}

void
SaveObject::end()
{
    //ASSERT(m_propertyIndex == obj_header[3]);
    afl::io::Stream::FileSize_t end_pos = m_stream.getPos();
    m_header.size = uint32_t(end_pos - m_headerPosition - 4*4);
    m_stream.setPos(m_headerPosition);
    writeHeader();
    m_stream.setPos(end_pos);
}

void
SaveObject::startProperty(uint32_t count)
{
    m_thisPropertyPosition = m_stream.getPos();
    if (UInt32_t* p = m_properties.at(2*m_propertyIndex)) {
        *p = count;
    }
}

void
SaveObject::endProperty()
{
    if (UInt32_t* p = m_properties.at(2*m_propertyIndex + 1)) {
        *p = uint32_t(m_stream.getPos() - m_thisPropertyPosition);
    }
    ++m_propertyIndex;
}

/**************************** FileSaveContext ****************************/

// Constructor.
interpreter::vmio::FileSaveContext::FileSaveContext(afl::charset::Charset& cs)
    : m_charset(cs),
      m_debugInformationEnabled(true),
      m_objectToId(),
      m_objectIdCounter(0),
      m_plan()
{
    // ex IntVMSaveContext::IntVMSaveContext
}

// Destructor.
interpreter::vmio::FileSaveContext::~FileSaveContext()
{ }

// Enable/disable debug information.
void
interpreter::vmio::FileSaveContext::setDebugInformation(bool enable)
{
    m_debugInformationEnabled = enable;
}

// Get number of prepared objects.
size_t
interpreter::vmio::FileSaveContext::getNumPreparedObjects() const
{
    return m_plan.size();
}

// Add process object.
void
interpreter::vmio::FileSaveContext::addProcess(Process& proc)
{
    // ex IntVMSaveContext::addProcess
    class ProcessSaver : public Saver {
     public:
        ProcessSaver(Process& p)
            : m_process(p)
            { }
        virtual void save(afl::io::Stream& out, FileSaveContext& parent)
            { parent.saveProcess(out, m_process); }
     private:
        Process& m_process;
    };
    addPlanNew(new ProcessSaver(proc));
}

// Save as object file.
void
interpreter::vmio::FileSaveContext::saveObjectFile(afl::io::Stream& out, uint32_t entry)
{
    // Save header
    structures::ObjectFileHeader header;
    std::memcpy(header.magic, structures::OBJECT_FILE_MAGIC, sizeof(header.magic));
    header.version = structures::OBJECT_FILE_VERSION;
    header.zero = 0;
    header.headerSize = structures::OBJECT_FILE_HEADER_SIZE;
    header.entry = entry;
    out.fullWrite(afl::base::fromObject(header));

    // Save content
    save(out);
}

// Save all pending objects.
void
interpreter::vmio::FileSaveContext::save(afl::io::Stream& out)
{
    // ex IntVMSaveContext::save
    for (size_t i = 0, n = m_plan.size(); i < n; ++i) {
        m_plan[i]->save(out, *this);
    }
}

// SaveContext:
uint32_t
interpreter::vmio::FileSaveContext::addBCO(const BytecodeObject& bco)
{
    // ex IntVMSaveContext::addBCO
    class BCOSaver : public Saver {
     public:
        BCOSaver(const BytecodeObject& bco, uint32_t id)
            : m_bco(bco), m_id(id)
            { }
        virtual void save(afl::io::Stream& out, FileSaveContext& parent)
            { parent.saveBCO(out, m_bco, m_id); }
     private:
        const BytecodeObject& m_bco;
        uint32_t m_id;
    };

    uint32_t& id = m_objectToId[&bco];
    if (id == 0) {
        id = ++m_objectIdCounter;
        addPlanNew(new BCOSaver(bco, id));
    }
    return id;
}

uint32_t
interpreter::vmio::FileSaveContext::addHash(const afl::data::Hash& hash)
{
    // ex IntVMSaveContext::addHash
    class HashSaver : public Saver {
     public:
        HashSaver(const afl::data::Hash& hash, uint32_t id)
            : m_hash(hash), m_id(id)
            { }
        virtual void save(afl::io::Stream& out, FileSaveContext& parent)
            { parent.saveHash(out, m_hash, m_id); }
     private:
        const afl::data::Hash& m_hash;
        uint32_t m_id;
    };

    uint32_t& id = m_objectToId[&hash];
    if (id == 0) {
        id = ++m_objectIdCounter;
        addPlanNew(new HashSaver(hash, id));
    }
    return id;
}

uint32_t
interpreter::vmio::FileSaveContext::addArray(const ArrayData& array)
{
    // ex IntVMSaveContext::addArray
    class ArraySaver : public Saver {
     public:
        ArraySaver(const ArrayData& array, uint32_t id)
            : m_array(array), m_id(id)
            { }
        virtual void save(afl::io::Stream& out, FileSaveContext& parent)
            { parent.saveArray(out, m_array, m_id); }
     private:
        const ArrayData& m_array;
        uint32_t m_id;
    };

    uint32_t& id = m_objectToId[&array];
    if (id == 0) {
        id = ++m_objectIdCounter;
        addPlanNew(new ArraySaver(array, id));
    }
    return id;
}

uint32_t
interpreter::vmio::FileSaveContext::addStructureType(const StructureTypeData& type)
{
    // ex IntVMSaveContext::addStructureType
    class StructureTypeSaver : public Saver {
     public:
        StructureTypeSaver(const StructureTypeData& type, uint32_t id)
            : m_type(type), m_id(id)
            { }
        virtual void save(afl::io::Stream& out, FileSaveContext& parent)
            { parent.saveStructureType(out, m_type, m_id); }
     private:
        const StructureTypeData& m_type;
        uint32_t m_id;
    };

    uint32_t& id = m_objectToId[&type];
    if (id == 0) {
        id = ++m_objectIdCounter;
        addPlanNew(new StructureTypeSaver(type, id));
    }
    return id;
}

uint32_t
interpreter::vmio::FileSaveContext::addStructureValue(const StructureValueData& value)
{
    // ex IntVMSaveContext::addStructureValue
    class StructureValueSaver : public Saver {
     public:
        StructureValueSaver(const StructureValueData& value, uint32_t id)
            : m_value(value), m_id(id)
            { }
        virtual void save(afl::io::Stream& out, FileSaveContext& parent)
            { parent.saveStructureValue(out, m_value, m_id); }
     private:
        const StructureValueData& m_value;
        uint32_t m_id;
    };

    uint32_t& id = m_objectToId[&value];
    if (id == 0) {
        id = ++m_objectIdCounter;
        addPlanNew(new StructureValueSaver(value, id));
    }
    return id;
}

bool
interpreter::vmio::FileSaveContext::isCurrentProcess(const Process* /*p*/)
{
    return false;
}

void
interpreter::vmio::FileSaveContext::addPlanNew(Saver* p)
{
    std::auto_ptr<Saver> pp(p);

    // Save preconditions
    afl::io::NullStream ns;
    pp->save(ns, *this);

    // Remember the plan
    m_plan.pushBackNew(pp.release());
}

// Save a bytecode object.
void
interpreter::vmio::FileSaveContext::saveBCO(afl::io::Stream& out, const BytecodeObject& bco, uint32_t id)
{
    // ex IntVMSaveContext::saveBCO
    SaveObject so(out);
    so.start(structures::otyp_Bytecode, id, 8);

    // Property 1: header (num_labels, flags, min_args, max_args)
    structures::BCOHeader header;
    so.startProperty(0);
    header.flags = uint16_t((bco.isProcedure() * header.ProcedureFlag) + (bco.isVarargs() * header.VarargsFlag));
    header.minArgs = uint16_t(bco.getMinArgs());
    header.maxArgs = uint16_t(bco.getMaxArgs());
    header.numLabels = bco.getNumLabels();
    out.fullWrite(afl::base::fromObject(header));
    so.endProperty();

    // Property 2: "data" (literals for pushlit, data segment)
    const afl::data::Segment& literals = bco.getLiterals();
    so.startProperty(uint32_t(literals.size()));
    SaveVisitor::save(out, literals, literals.size(), m_charset, *this);
    so.endProperty();

    // Property 3: "names" (names for e.g. pushvar, name list)
    const afl::data::NameMap& names = bco.getNames();
    so.startProperty(uint32_t(names.getNumNames()));
    SaveVisitor::saveNames(out, names, names.getNumNames(), m_charset);
    so.endProperty();

    // Property 4: "code" (count = number of instructions, size = 4x count). 32 bit per instruction.
    const std::vector<Opcode>& code = bco.getCode();
    so.startProperty(uint32_t(code.size()));
    writeArray32(out, afl::base::Memory<const Opcode>(code));
    so.endProperty();

    // Property 5: "local_names" (predeclared locals, name list)
    const afl::data::NameMap& localNames = bco.getLocalNames();
    so.startProperty(uint32_t(localNames.getNumNames()));
    SaveVisitor::saveNames(out, localNames, localNames.getNumNames(), m_charset);
    so.endProperty();

    // Property 6: "name" (name hint for loading, string)
    so.startProperty(0);
    out.fullWrite(afl::string::toBytes(bco.getName()));
    so.endProperty();

    // Property 7: "file name" (debug file name, string)
    so.startProperty(0);
    if (m_debugInformationEnabled) {
        out.fullWrite(afl::string::toBytes(bco.getFileName()));
    }
    so.endProperty();

    // Property 8: "line numbers" (count = number of lines, size = 8x count)
    const std::vector<uint32_t>& lineNumbers = bco.getLineNumbers();
    if (m_debugInformationEnabled) {
        so.startProperty(uint32_t(lineNumbers.size()/2));
        writeArray32(out, afl::base::Memory<const uint32_t>(lineNumbers));
        so.endProperty();
    } else {
        so.startProperty(0);
        so.endProperty();
    }

    so.end();
}

// Save a hash object.
void
interpreter::vmio::FileSaveContext::saveHash(afl::io::Stream& out, const afl::data::Hash& hash, uint32_t id)
{
    // ex IntVMSaveContext::saveHash
    SaveObject so(out);
    so.start(structures::otyp_DataHash, id, 2);

    // Property 1: names
    const afl::data::NameMap& names = hash.getKeys();
    so.startProperty(uint32_t(names.getNumNames()));
    SaveVisitor::saveNames(out, names, names.getNumNames(), m_charset);
    so.endProperty();

    // Property 2: values
    const afl::data::Segment& content = hash.getValues();
    so.startProperty(uint32_t(content.size()));
    SaveVisitor::save(out, content, content.size(), m_charset, *this);
    so.endProperty();

    so.end();
}

// Save an array object.
void
interpreter::vmio::FileSaveContext::saveArray(afl::io::Stream& out, const ArrayData& array, uint32_t id)
{
    // ex IntVMSaveContext::saveArray
    SaveObject so(out);
    so.start(structures::otyp_DataArray, id, 2);

    // Property 1: dimensions
    const std::vector<size_t>& dim = array.getDimensions();
    so.startProperty(uint32_t(dim.size()));
    writeArray32(out, afl::base::Memory<const size_t>(dim));
    so.endProperty();

    // Property 2: content
    const afl::data::Segment& content = array.content;
    so.startProperty(uint32_t(content.size()));
    SaveVisitor::save(out, content, content.size(), m_charset, *this);
    so.endProperty();

    so.end();
}

// Save a structure type.
void
interpreter::vmio::FileSaveContext::saveStructureType(afl::io::Stream& out, const StructureTypeData& type, uint32_t id)
{
    // ex IntVMSaveContext::saveStructureType
    SaveObject so(out);
    so.start(structures::otyp_DataStructType, id, 1);

    // Property 1: name list
    const afl::data::NameMap& names = type.names();
    so.startProperty(uint32_t(names.getNumNames()));
    SaveVisitor::saveNames(out, names, names.getNumNames(), m_charset);
    so.endProperty();

    so.end();
}

// Save a structure value.
void
interpreter::vmio::FileSaveContext::saveStructureValue(afl::io::Stream& out, const StructureValueData& value, uint32_t id)
{
    // ex IntVMSaveContext::saveStructureValue
    SaveObject so(out);
    so.start(structures::otyp_DataStructValue, id, 2);

    // Property 1: header
    so.startProperty(0);
    UInt32_t header[1];
    header[0] = addStructureType(*value.type);
    out.fullWrite(afl::base::fromObject(header));
    so.endProperty();

    // Property 2: content
    const afl::data::Segment& data = value.data;
    so.startProperty(uint32_t(data.size()));
    SaveVisitor::save(out, data, data.size(), m_charset, *this);
    so.endProperty();

    so.end();
}

// Save a stack frame object.
void
interpreter::vmio::FileSaveContext::saveFrame(afl::io::Stream& out, const Process::Frame& fr)
{
    // ex IntVMSaveContext::saveFrame
    SaveObject so(out);

    // We don't actually need the frame_sp here (it will be ignored and
    // reconstructed upon load), but it doesn't hurt.
    so.start(structures::otyp_Frame, uint32_t(fr.frameSP), 3);

    // Property 1: header
    so.startProperty(0);
    structures::FrameHeader header;
    header.bcoRef = addBCO(*fr.bco);
    header.pc = uint32_t(fr.pc);
    header.contextSP = uint32_t(fr.contextSP);
    header.exceptionSP = uint32_t(fr.exceptionSP);
    header.flags = (fr.wantResult * header.WantResult);
    out.fullWrite(afl::base::fromObject(header));
    so.endProperty();

    // Property 2: local values (data segment)
    so.startProperty(uint32_t(fr.localValues.size()));
    SaveVisitor::save(out, fr.localValues, fr.localValues.size(), m_charset, *this);
    so.endProperty();

    // Property 3: local names (name list)
    so.startProperty(uint32_t(fr.localNames.getNumNames()));
    SaveVisitor::saveNames(out, fr.localNames, fr.localNames.getNumNames(), m_charset);
    so.endProperty();

    so.end();
}

// Save process.
void
interpreter::vmio::FileSaveContext::saveProcess(afl::io::Stream& out, const Process& proc)
{
    // Nested context to provide process context to mutexes
    ProcessSaveContext childContext(*this, proc);

    // Start the object
    SaveObject so(out);
    so.start(structures::otyp_Process, 0, 6);

    // Property 1: header
    so.startProperty(0);
    structures::ProcessHeader header;
    header.priority = uint8_t(proc.getPriority());
    header.kind = proc.getProcessKind();
    header.contextTOS = uint16_t(proc.getContextTOS());
    out.fullWrite(afl::base::fromObject(header));
    so.endProperty();

    // Property 2: name (string)
    so.startProperty(0);
    out.fullWrite(afl::string::toBytes(proc.getName()));
    so.endProperty();

    // Property 3: frames (object array)
    size_t numFrames = proc.getNumActiveFrames();
    so.startProperty(uint32_t(numFrames));
    for (size_t i = 0; i < numFrames; ++i) {
        if (const Process::Frame* f = proc.getFrame(i)) {
            // FIXME: do we need to use childContext here?
            saveFrame(out, *f);
        }
    }
    so.endProperty();

    // Property 4: contexts (data segment)
    const afl::container::PtrVector<Context>& contexts = proc.getContexts();
    so.startProperty(uint32_t(contexts.size()));
    SaveVisitor::saveContexts(out, contexts, m_charset, childContext);
    so.endProperty();

    // Property 5: exceptions (counts = number, size = 16xcount)
    const afl::container::PtrVector<Process::ExceptionHandler>& exceptions = proc.getExceptions();
    so.startProperty(uint32_t(exceptions.size()));
    for (size_t i = 0, n = exceptions.size(); i < n; ++i) {
        UInt32_t tmp[4];
        tmp[0] = uint32_t(exceptions[i]->frameSP);
        tmp[1] = uint32_t(exceptions[i]->contextSP);
        tmp[2] = uint32_t(exceptions[i]->valueSP);
        tmp[3] = uint32_t(exceptions[i]->pc);
        out.fullWrite(afl::base::fromObject(tmp));
    }
    so.endProperty();

    // Property 6: value stack (data segment)
    const Process::Segment_t& valueStack = proc.getValueStack();
    so.startProperty(uint32_t(valueStack.size()));
    SaveVisitor::save(out, valueStack, valueStack.size(), m_charset, childContext);
    so.endProperty();

    // Finish
    so.end();
}
