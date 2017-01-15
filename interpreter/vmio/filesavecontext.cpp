/**
  *  \file interpreter/vmio/filesavecontext.cpp
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
#include "afl/bits/pack.hpp"
#include "afl/io/nullstream.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/hashdata.hpp"
#include "interpreter/savevisitor.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/vmio/processsavecontext.hpp"
#include "interpreter/vmio/structures.hpp"
#include "afl/base/growablememory.hpp"

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

    template<typename T>
    void
    writeArray32(afl::io::Stream& out, interpreter::vmio::FileSaveContext& ctx, afl::base::Memory<const T> obj)
    {
        const size_t CHUNK = 128;
        while (!obj.empty()) {
            UInt32_t buffer[CHUNK];
            afl::base::Memory<const T> now(obj.split(CHUNK));
            size_t i = 0;
            while (const T* p = now.eat()) {
                buffer[i++] = ctx.toWord(*p);
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
    m_header.size = end_pos - m_headerPosition - 4*4;
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
        *p = m_stream.getPos() - m_thisPropertyPosition;
    }
    ++m_propertyIndex;
}


interpreter::vmio::FileSaveContext::FileSaveContext(afl::charset::Charset& cs)
    : m_charset(cs),
      m_debugInformationEnabled(true),
      obj_to_id(),
      obj_id_counter(0),
      plan_objs(),
      plan_types()
{
    // ex IntVMSaveContext::IntVMSaveContext
}

interpreter::vmio::FileSaveContext::~FileSaveContext()
{ }

void
interpreter::vmio::FileSaveContext::setDebugInformation(bool enable)
{
    m_debugInformationEnabled = enable;
}

size_t
interpreter::vmio::FileSaveContext::getNumPreparedObjects() const
{
    return plan_objs.size();
}

// /** Add process object to serialization. Note that, unlike bytecode, a process
//     can be saved many times. Loading will then create multiple copies of it.
//     \param proc The process object */
void
interpreter::vmio::FileSaveContext::addProcess(Process& proc)
{
    // ex IntVMSaveContext::addProcess
    /* Prepare it by saving into a temporary memory stream: this
       causes its preconditions (=BCOs) to be saved */
    afl::io::NullStream ns;
    saveProcess(ns, proc);

    /* Remember the plan */
    plan_objs.push_back(&proc);
    plan_types.push_back(poProcess);
}

// /** Save all pending objects.
//     \param out Stream to save to */
void
interpreter::vmio::FileSaveContext::save(afl::io::Stream& out)
{
    // ex IntVMSaveContext::save
    // ASSERT(plan_objs.size() == plan_types.size());
    for (size_t i = 0; i < plan_objs.size(); ++i) {
        switch (plan_types[i]) {
         case poBytecode:
            saveBCO(out, *static_cast<BytecodeObject*>(plan_objs[i]), obj_to_id[plan_objs[i]]);
            break;
         case poProcess:
            saveProcess(out, *static_cast<Process*>(plan_objs[i]));
            break;
         case poArray:
            saveArray(out, *static_cast<ArrayData*>(plan_objs[i]), obj_to_id[plan_objs[i]]);
            break;
         case poHash:
            saveHash(out, *static_cast<HashData*>(plan_objs[i]), obj_to_id[plan_objs[i]]);
            break;
         case poStructType:
            saveStructureType(out, *static_cast<StructureTypeData*>(plan_objs[i]), obj_to_id[plan_objs[i]]);
            break;
         case poStructValue:
            saveStructureValue(out, *static_cast<StructureValueData*>(plan_objs[i]), obj_to_id[plan_objs[i]]);
            break;
        }
    }
}

uint32_t
interpreter::vmio::FileSaveContext::toWord(uint32_t value)
{
    // ex int/vmio.h:storeItem
    return value;
}

uint32_t
interpreter::vmio::FileSaveContext::toWord(Opcode insn)
{
    // ex int/vmio.h:storeItem
    return uint32_t(insn.arg)
        + (uint32_t(insn.minor) << 16)
        + (uint32_t(insn.getExternalMajor()) << 24);
}

uint32_t
interpreter::vmio::FileSaveContext::toWord(BCORef_t bco)
{
    // ex int/vmio.h:storeItem
    return addBCO(*bco);
}


// SaveContext:
// /** Add bytecode object to serialization. This assigns the BCO an Id
//     number which can be used to refer to that BCO. The Id number is
//     returned. Adding an object multiple times only saves it once.
//     \param bco the bytecode object
//     \return Id number */
uint32_t
interpreter::vmio::FileSaveContext::addBCO(BytecodeObject& bco)
{
    // ex IntVMSaveContext::addBCO
    /* Is this item already known? */
    uint32_t& id = obj_to_id[&bco];
    if (id == 0) {
        /* This object is not yet known. Give it an Id. */
        id = ++obj_id_counter;

        /* Save its preconditions. Note that if the BCO indirectly refers
           to itself, the nested addBCO will see that it already has an
           Id (although it is not yet planned) and just re-use that. */
        afl::io::NullStream ns;
        saveBCO(ns, bco, id);

        /* Remember the plan */
        plan_objs.push_back(&bco);
        plan_types.push_back(poBytecode);
    }
    return id;
}

// /** Add hash object to serialisation. This assigns the hash an
//     Id number which can be used to refer to that hash. The Id
//     number is returned. Adding an object multiple times only saves
//     it once.
//     \param hash the hash data
//     \return Id number */
uint32_t
interpreter::vmio::FileSaveContext::addHash(HashData& hash)
{
    // ex IntVMSaveContext::addHash
    /* Is this item already known? */
    uint32_t& id = obj_to_id[&hash];
    if (id == 0) {
        /* This object is not yet known. Give it an Id. */
        id = ++obj_id_counter;

        /* Save its preconditions */
        afl::io::NullStream ns;
        saveHash(ns, hash, id);

        /* Remember the plan */
        plan_objs.push_back(&hash);
        plan_types.push_back(poHash);
    }
    return id;
}

// /** Add array object to serialisation. This assigns the array an
//     Id number which can be used to refer to that array. The Id
//     number is returned. Adding an object multiple times only saves
//     it once.
//     \param array the array data
//     \return Id number */
uint32_t
interpreter::vmio::FileSaveContext::addArray(ArrayData& array)
{
    // ex IntVMSaveContext::addArray
    /* Is this item already known? */
    uint32_t& id = obj_to_id[&array];
    if (id == 0) {
        /* This object is not yet known. Give it an Id. */
        id = ++obj_id_counter;

        /* Save its preconditions */
        afl::io::NullStream ns;
        saveArray(ns, array, id);

        /* Remember the plan */
        plan_objs.push_back(&array);
        plan_types.push_back(poArray);
    }
    return id;
}

// /** Add structure type object to serialisation. This assigns the
//     type an Id number which can be used to refer to that type.
//     The Id number is returned. Adding an object multiple times
//     only saves it once.
//     \param array the structure type
//     \return Id number */
uint32_t
interpreter::vmio::FileSaveContext::addStructureType(StructureTypeData& type)
{
    // ex IntVMSaveContext::addStructureType
    /* Is this item already known? */
    uint32_t& id = obj_to_id[&type];
    if (id == 0) {
        /* This object is not yet known. Give it an Id. */
        id = ++obj_id_counter;

        /* Save its preconditions */
        afl::io::NullStream ns;
        saveStructureType(ns, type, id);

        /* Remember the plan */
        plan_objs.push_back(&type);
        plan_types.push_back(poStructType);
    }
    return id;
}

// /** Add structure value object to serialisation. This assigns the
//     value an Id number which can be used to refer to it. The Id
//     number is returned. Adding an object multiple times only saves
//     it once.
//     \param array the structure value
//     \return Id number */
uint32_t
interpreter::vmio::FileSaveContext::addStructureValue(StructureValueData& value)
{
    // ex IntVMSaveContext::addStructureValue
    /* Is this item already known? */
    uint32_t& id = obj_to_id[&value];
    if (id == 0) {
        /* This object is not yet known. Give it an Id. */
        id = ++obj_id_counter;

        /* Save its preconditions */
        afl::io::NullStream ns;
        saveStructureValue(ns, value, id);

        /* Remember the plan */
        plan_objs.push_back(&value);
        plan_types.push_back(poStructValue);
    }
    return id;
}

bool
interpreter::vmio::FileSaveContext::isCurrentProcess(Process* /*p*/)
{
    return false;
}


// /** Save a bytecode object.
//     \param out  Stream to write to
//     \param p    Bytecode object to save
//     \param id   Id to assign this BCO */
void
interpreter::vmio::FileSaveContext::saveBCO(afl::io::Stream& out, const BytecodeObject& bco, uint32_t id)
{
    // ex IntVMSaveContext::saveBCO
    SaveObject so(out);
    so.start(structures::otyp_Bytecode, id, 8);

    // Property 1: header (num_labels, flags, min_args, max_args)
    structures::BCOHeader header;
    so.startProperty(0);
    header.flags = (bco.isProcedure() * header.ProcedureFlag) + (bco.isVarargs() * header.VarargsFlag);
    header.minArgs = bco.getMinArgs();
    header.maxArgs = bco.getMaxArgs();
    header.numLabels = bco.getNumLabels();
    out.fullWrite(afl::base::fromObject(header));
    so.endProperty();

    // Property 2: "data" (literals for pushlit, data segment)
    const afl::data::Segment& literals = bco.getLiterals();
    so.startProperty(literals.size());
    SaveVisitor::save(out, literals, literals.size(), m_charset, *this);
    so.endProperty();

    // Property 3: "names" (names for e.g. pushvar, name list)
    const afl::data::NameMap& names = bco.getNames();
    so.startProperty(names.getNumNames());
    SaveVisitor::saveNames(out, names, names.getNumNames(), m_charset);
    so.endProperty();

    // Property 4: "code" (count = number of instructions, size = 4x count). 32 bit per instruction.
    const std::vector<Opcode>& code = bco.getCode();
    so.startProperty(code.size());
    writeArray32(out, *this, afl::base::Memory<const Opcode>(code));
    so.endProperty();

    // Property 5: "local_names" (predeclared locals, name list)
    const afl::data::NameMap& localNames = bco.getLocalNames();
    so.startProperty(localNames.getNumNames());
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
        so.startProperty(lineNumbers.size()/2);
        writeArray32(out, *this, afl::base::Memory<const uint32_t>(lineNumbers));
        so.endProperty();
    } else {
        so.startProperty(0);
        so.endProperty();
    }

    so.end();
}

// /** Save a hash object.
//     \param out Stream to save to
//     \param hash Hash to save
//     \param id Id to use */
void
interpreter::vmio::FileSaveContext::saveHash(afl::io::Stream& out, const HashData& hash, uint32_t id)
{
    // ex IntVMSaveContext::saveHash
    SaveObject so(out);
    so.start(structures::otyp_DataHash, id, 2);

    // Property 1: names
    const afl::data::NameMap& names = hash.getNames();
    so.startProperty(names.getNumNames());
    SaveVisitor::saveNames(out, names, names.getNumNames(), m_charset);
    so.endProperty();

    // Property 2: values
    const afl::data::Segment& content = hash.getContent();
    so.startProperty(content.size());
    SaveVisitor::save(out, content, content.size(), m_charset, *this);
    so.endProperty();

    so.end();
}

// /** Save an array object.
//     \param out Stream to save to
//     \param array Array to save
//     \param id Id to use */
void
interpreter::vmio::FileSaveContext::saveArray(afl::io::Stream& out, const ArrayData& array, uint32_t id)
{
    // ex IntVMSaveContext::saveArray
    SaveObject so(out);
    so.start(structures::otyp_DataArray, id, 2);

    // Property 1: dimensions
    const std::vector<size_t>& dim = array.getDimensions();
    so.startProperty(dim.size());
    writeArray32(out, *this, afl::base::Memory<const size_t>(dim));
    so.endProperty();

    // Property 2: content
    const afl::data::Segment& content = array.content;
    so.startProperty(content.size());
    SaveVisitor::save(out, content, content.size(), m_charset, *this);
    so.endProperty();

    so.end();
}

// /** Save a structure type.
//     \param out Stream to save to
//     \param type Type to save
//     \param id Id to use */
void
interpreter::vmio::FileSaveContext::saveStructureType(afl::io::Stream& out, const StructureTypeData& type, uint32_t id)
{
    // ex IntVMSaveContext::saveStructureType
    SaveObject so(out);
    so.start(structures::otyp_DataStructType, id, 1);

    // Property 1: name list
    const afl::data::NameMap& names = type.names;
    so.startProperty(names.getNumNames());
    SaveVisitor::saveNames(out, names, names.getNumNames(), m_charset);
    so.endProperty();

    so.end();
}

// /** Save a structure value.
//     \param out Stream to save to
//     \param value Value to save
//     \param id Id to use */
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
    so.startProperty(data.size());
    SaveVisitor::save(out, data, data.size(), m_charset, *this);
    so.endProperty();

    so.end();
}

// /** Save a stack frame object.
//     \param out Stream to save to
//     \param fr  Stack frame to save */
void
interpreter::vmio::FileSaveContext::saveFrame(afl::io::Stream& out, const Process::Frame& fr)
{
    // ex IntVMSaveContext::saveFrame
    SaveObject so(out);

    // We don't actually need the frame_sp here (it will be ignored and
    // reconstructed upon load), but it doesn't hurt.
    so.start(structures::otyp_Frame, fr.frameSP, 3);

    // Property 1: header
    so.startProperty(0);
    structures::FrameHeader header;
    header.bcoRef = addBCO(*fr.bco);
    header.pc = fr.pc;
    header.contextSP = fr.contextSP;
    header.exceptionSP = fr.exceptionSP;
    header.flags = (fr.wantResult * header.WantResult);
    out.fullWrite(afl::base::fromObject(header));
    so.endProperty();

    // Property 2: local values (data segment)
    so.startProperty(fr.localValues.size());
    SaveVisitor::save(out, fr.localValues, fr.localValues.size(), m_charset, *this);
    so.endProperty();

    // Property 3: local names (name list)
    so.startProperty(fr.localNames.getNumNames());
    SaveVisitor::saveNames(out, fr.localNames, fr.localNames.getNumNames(), m_charset);
    so.endProperty();

    so.end();
}

// /** Save process.
//     \param out Stream to save to
//     \param exc Process to save */
void
interpreter::vmio::FileSaveContext::saveProcess(afl::io::Stream& out, Process& proc)
{
    // Nested context to provide process context to mutexes
    ProcessSaveContext childContext(*this, proc);

    // Start the object
    SaveObject so(out);
    so.start(structures::otyp_Process, 0, 6);

    // Property 1: header
    so.startProperty(0);
    structures::ProcessHeader header;
    header.priority = proc.getPriority();
    header.kind = proc.getProcessKind();
    header.contextTOS = proc.getContextTOS();
    out.fullWrite(afl::base::fromObject(header));
    so.endProperty();

    // Property 2: name (string)
    so.startProperty(0);
    out.fullWrite(afl::string::toBytes(proc.getName()));
    so.endProperty();

    // Property 3: frames (object array)
    size_t numFrames = proc.getNumActiveFrames();
    so.startProperty(numFrames);
    for (size_t i = 0; i < numFrames; ++i) {
        if (const Process::Frame* f = proc.getFrame(i)) {
            // FIXME: do we need to use childContext here?
            saveFrame(out, *f);
        }
    }
    so.endProperty();

    // Property 4: contexts (data segment)
    const afl::container::PtrVector<Context>& contexts = proc.getContexts();
    so.startProperty(contexts.size());
    SaveVisitor::saveContexts(out, contexts, m_charset, childContext);
    so.endProperty();

    // Property 5: exceptions (counts = number, size = 16xcount)
    const afl::container::PtrVector<Process::ExceptionHandler>& exceptions = proc.getExceptions();
    so.startProperty(exceptions.size());
    for (size_t i = 0, n = exceptions.size(); i < n; ++i) {
        UInt32_t tmp[4];
        tmp[0] = exceptions[i]->frameSP;
        tmp[1] = exceptions[i]->contextSP;
        tmp[2] = exceptions[i]->valueSP;
        tmp[3] = exceptions[i]->pc;
        out.fullWrite(afl::base::fromObject(tmp));
    }
    so.endProperty();

    // Property 6: value stack (data segment)
    const Process::Segment_t& valueStack = proc.getValueStack();
    so.startProperty(valueStack.size());
    SaveVisitor::save(out, valueStack, valueStack.size(), m_charset, childContext);
    so.endProperty();

    // Finish
    so.end();
}
