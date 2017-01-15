/**
  *  \file interpreter/bytecodeobject.hpp
  */
#ifndef C2NG_INTERPRETER_BYTECODEOBJECT_HPP
#define C2NG_INTERPRETER_BYTECODEOBJECT_HPP

#include <vector>
#include "afl/base/ptr.hpp"
#include "interpreter/opcode.hpp"
#include "afl/data/value.hpp"
#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"

namespace interpreter {

//   *  \todo Can we make referred BCOs literals? pro: this would save a special
//   *  case and an addressing mode. con: we would need to be able to serialize
//   *  BCOs in IntDataSegment.

    class World;
    class BytecodeObject;
    class CompilationContext; // FIXME? was 'struct'

// class IntVMSaveContext;
// class IntVMLoadContext;

    /** Reference to a BytecodeObject. */
    typedef afl::base::Ref<BytecodeObject> BCORef_t;
    typedef afl::base::Ptr<BytecodeObject> BCOPtr_t;

// /** Bytecode object (BCO). Bytecode objects contain code for execution.
//     It contains the following elements:
//     - actual code
//     - a list of literals referred to by the code (only small integer literals
//       can be encoded directly in the code, others are loaded from this table).
//     - a list of names (symbols) referred to by the code.
//     - a list of other BCOs referred to by the code. This means only subroutines
//       <em>defined</em> by this BCO, not <em>called</em> subroutines.
//     - a list of predeclared identifiers
//     - additional information about this BCO as a subroutine ("is procedure"
//       flag, argument counts).
//     - optional additional information about this BCO's source code (aka debug
//       information), i.e. file name and line/address associations. We assume
//       that each BCO is compiled from a single file, that is, we don't have an
//       "#include"-style preprocessor, but a "Load" instruction executed at
//       runtime.

//     For the benefit of code generation, the BCO can also contain symbolic
//     labels.

//     BCOs use reference counting. Each IntExecutionFrame executing a BCO refers
//     to it, as well as the symbol table entry holding it, as well as another
//     BCO which happened to define it. Circular references are impossible. For
//     example, assume we are executing "foo" and are at the line containing "bar":
//     <pre>
//       Sub foo
//         Sub bar
//           Print "hi"
//         EndSub
//         bar
//       EndSub
//     </pre>
//     The BCO "foo" will be referenced from the symbol table and the executing
//     frame. The BCO "bar" will be referenced from the symbol table (it has just
//     been defined), from the executing frame, and from the "foo" BCO.

//     BCOs contain a list of predeclared identifiers (see getLocalNames). These
//     are the names of the parameters used to invoke the parameters, plus the
//     names of predeclared local variables. Predeclaring a variable will give it
//     a known address at compile time, allowing faster code to be generated.
//     The compiler will use this instead of a <code>dimloc</code> instruction
//     when it can prove that it's safe to do so. */
    class BytecodeObject : public afl::base::RefCounted {
//     friend class IntVMSaveContext;
//     friend class IntVMLoadContext;
     public:
        typedef std::vector<Opcode>::size_type PC_t;
        typedef uint16_t Label_t;

        BytecodeObject();
        ~BytecodeObject();

        void     addArgument(String_t name, bool optional);
        uint16_t addLocalVariable(const String_t& name);
        bool     hasLocalVariable(const String_t& name);
        void     setIsProcedure(bool flag);
        void     setIsVarargs(bool flag);
        size_t   getMinArgs() const;
        size_t   getMaxArgs() const;
        void     setMinArgs(size_t n);
        void     setMaxArgs(size_t n);
        bool     isProcedure() const;
        bool     isVarargs() const;
        String_t getName() const;
        void     setName(String_t name);
        String_t getFileName() const;
        void     setFileName(String_t fileName);
        void     addLineNumber(uint32_t line);
        void     addLineNumber(uint32_t line, uint32_t pc);
        uint32_t getLineNumber(PC_t pc) const;

        Label_t  makeLabel();
        void     addInstruction(Opcode::Major major, uint8_t minor, uint16_t arg);
        void     addVariableReferenceInstruction(Opcode::Major major, const String_t& name, const CompilationContext& cc);
        void     addLabel(Label_t label);
        void     addJump(uint8_t flags, Label_t label);
        void     addPushLiteral(afl::data::Value* literal);
        uint16_t addName(String_t name);
        bool     hasName(String_t name) const;
        bool     hasUserCall() const;
        void     relocate();
        void     compact();
        void     copyLocalVariablesFrom(const BytecodeObject& other);
        void     append(const BytecodeObject& other);

        PC_t     getNumInstructions() const;
        uint32_t getNumLabels() const;
        void     setNumLabels(uint32_t n);
        PC_t     getJumpTarget(uint8_t minor, uint16_t arg) const;
        Opcode&  operator()(PC_t index);
        const Opcode& operator()(PC_t index) const;
        String_t getDisassembly(PC_t index, const World& w) const;

        afl::data::Value* getLiteral(uint16_t index) const;
        const String_t& getName(uint16_t index) const;
        const afl::data::NameMap& getLocalNames() const;  // FIXME: rename to getLocalVariableNames
        afl::data::NameMap& getLocalNames()
            { return m_localNames; }

        const afl::data::NameMap& getNames() const
            { return m_names; }
        afl::data::NameMap& getNames()
            { return m_names; }

        const afl::data::Segment& getLiterals() const
            { return m_data; }
        afl::data::Segment& getLiterals()
            { return m_data; }

        const std::vector<Opcode>& getCode() const
            { return m_code; }
        // std::vector<Opcode>& getCode()
        //     { return m_code; }

        const std::vector<uint32_t>& getLineNumbers() const
            { return line_numbers; }
        // std::vector<uint32_t>& getLineNumbers()
        //     { return line_numbers; }

     private:
        afl::data::Segment    m_data;  ///< Literals referenced in bytecode.
        afl::data::NameMap    m_names; ///< Names referenced in bytecode.
        std::vector<Opcode>   m_code;  ///< Actual code.

        Label_t               num_labels;

        afl::data::NameMap    m_localNames;

        size_t                m_minArgs;
        size_t                m_maxArgs;
        bool                  m_isProcedure;
        bool                  m_isVarargs;
        String_t              m_name;
        String_t              file_name;
        std::vector<uint32_t> line_numbers; ///< Line numbers. Pairs of address,line.
    };
}

/**************************** Inline Functions ***************************/

namespace std {
    /** Total order for IntBCORef. This allows to make a std::map
        using IntBCORef's as key. We don't need any particular order,
        so we just use the address order. */
    template<>
    struct less<interpreter::BCORef_t> {
        bool operator()(interpreter::BCORef_t a, interpreter::BCORef_t b) const
            { return less<void*>()(&a.get(), &b.get()); }
    };
}

// /** Add local variable. addLocalVariable() should not be followed by addArgument(). */
inline uint16_t
interpreter::BytecodeObject::addLocalVariable(const String_t& name)
{
    // ex IntBytecodeObject::addLocalVariable
    return m_localNames.add(name);
}

// /** Check for local variable.
//     \param name Name to check */
inline bool
interpreter::BytecodeObject::hasLocalVariable(const String_t& name)
{
    // ex IntBytecodeObject::hasLocalVariable
    return m_localNames.getIndexByName(name) != m_localNames.nil;
}

// /** Set the "is procedure" flag. If set, the generated code is a procedure without
//     result, which leaves the stack as-is. If clear, the generated code is a function
//     which generates a single result on the stack. */
inline void
interpreter::BytecodeObject::setIsProcedure(bool flag)
{
    // ex IntBytecodeObject::setIsProcedure
    m_isProcedure = flag;
}

// /** Set the "is varargs" flag. If set, arguments exceeding the maximum number of
//     args are wrapped into an array. */
inline void
interpreter::BytecodeObject::setIsVarargs(bool flag)
{
    // ex IntBytecodeObject::setIsVarargs
    m_isVarargs = flag;
}

// /** Get minimum number of expected arguments. */
inline size_t
interpreter::BytecodeObject::getMinArgs() const
{
    // ex IntBytecodeObject::getMinArgs
    return m_minArgs;
}

// /** Get maximum number of expected arguments. */
inline size_t
interpreter::BytecodeObject::getMaxArgs() const
{
    // ex IntBytecodeObject::getMaxArgs
    return m_maxArgs;
}

inline void
interpreter::BytecodeObject::setMinArgs(size_t n)
{
    m_minArgs = n;
}

inline void
interpreter::BytecodeObject::setMaxArgs(size_t n)
{
    m_maxArgs = n;
}

// /** Get the "is procedure" flag. \see setIsProcedure */
inline bool
interpreter::BytecodeObject::isProcedure() const
{
    // ex IntBytecodeObject::isProcedure
    return m_isProcedure;
}

// /** Get the "is varargs" flag. \see setIsVarargs */
inline bool
interpreter::BytecodeObject::isVarargs() const
{
    // ex IntBytecodeObject::isVarargs
    return m_isVarargs;
}

// /** Make a new label for future reference. This label can be used in as many jumps
//     as needed (addJump), and must be placed exactly once using addLabel. */
inline interpreter::BytecodeObject::Label_t
interpreter::BytecodeObject::makeLabel()
{
    // ex IntBytecodeObject::makeLabel
    return num_labels++;
}

// /** Add a name for reference by later instructions. Existing names are recycled if
//     possible. */
inline uint16_t
interpreter::BytecodeObject::addName(String_t name)
{
    // ex IntBytecodeObject::addName
    return m_names.addMaybe(name);
}

// /** Check whether we already know the specified name. */
inline bool
interpreter::BytecodeObject::hasName(String_t name) const
{
    // ex IntBytecodeObject::hasName
    return m_names.getIndexByName(name) != m_names.nil;
}

// /** Get number of instructions in this bytecode object. */
inline interpreter::BytecodeObject::PC_t
interpreter::BytecodeObject::getNumInstructions() const
{
    // ex IntBytecodeObject::getNumInstructions
    return m_code.size();
}

// /** Get number of symbolic labels in this bytecode object. */
inline uint32_t
interpreter::BytecodeObject::getNumLabels() const
{
    // ex IntBytecodeObject::getNumLabels
    return num_labels;
}

inline void
interpreter::BytecodeObject::setNumLabels(uint32_t n)
{
    num_labels = n;
}

// /** Read/write access to instruction by address. */
inline interpreter::Opcode&
interpreter::BytecodeObject::operator()(PC_t index)
{
    return m_code[index];
}

// /** Read access to instruction by address. */
inline const interpreter::Opcode&
interpreter::BytecodeObject::operator()(PC_t index) const
{
    return m_code[index];
}

// /** Get local name list. These are the parameter names and the
//     predeclared local variables. */
inline const afl::data::NameMap&
interpreter::BytecodeObject::getLocalNames() const
{
    // ex IntBytecodeObject::getLocalNames
    return m_localNames;
}

// /** Get literal from literal table. */
inline afl::data::Value*
interpreter::BytecodeObject::getLiteral(uint16_t index) const
{
    // ex IntBytecodeObject::getLiteral
    // No range check required; it is done by Segment
    return m_data[index];
}

// /** Get name from name table. */
inline const String_t&
interpreter::BytecodeObject::getName(uint16_t index) const
{
    return m_names.getNameByIndex(index);
}

#endif
