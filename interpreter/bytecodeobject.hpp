/**
  *  \file interpreter/bytecodeobject.hpp
  *  \brief Class interpreter::BytecodeObject
  */
#ifndef C2NG_INTERPRETER_BYTECODEOBJECT_HPP
#define C2NG_INTERPRETER_BYTECODEOBJECT_HPP

#include <vector>
#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/value.hpp"
#include "interpreter/opcode.hpp"

namespace interpreter {

    class World;
    class BytecodeObject;
    class CompilationContext;

    /** Reference to a BytecodeObject. */
    typedef afl::base::Ref<BytecodeObject> BCORef_t;
    typedef afl::base::Ptr<BytecodeObject> BCOPtr_t;

    /** Bytecode object (BCO).

        Bytecode objects contain code for execution.
        They contain the following elements:
        - actual code
        - a list of literals referred to by the code
          (only small integer literals can be encoded directly in the code, others are loaded from this table).
        - a list of names (symbols) referred to by the code.
        - a list of other BCOs referred to by the code. This means only subroutines <em>defined</em> by this BCO, not <em>called</em> subroutines.
        - a list of predeclared identifiers
        - additional information about this BCO as a subroutine ("is procedure" flag, argument counts).
        - optional additional information about this BCO's source code (aka debug information),
          i.e. file name and line/address associations.
          We assume that each BCO is compiled from a single file, that is, we don't have an "#include"-style preprocessor,
          only a "Load" instruction executed at runtime that produces complete BCOs.

       For the benefit of code generation, the BCO can also contain symbolic labels.

       BCOs use reference counting. Each BCO is referenced by
       - frames it is executing in
       - SubroutineValue's referencing it, in particular, in the BCO that defines it
       - the symbol table (World::globalPropertyNames)
       Circular references are impossible to produce by the compiler.
       For example, assume we are executing "foo" and are at the line containing "bar":
         <pre>
           Sub foo
             Sub bar
               Print "hi"
             EndSub
             bar
           EndSub
         </pre>
       The BCO "foo" will be referenced from the symbol table and the executing frame.
       The BCO "bar" will be referenced from the symbol table (it has just been defined),
       from the executing frame, and from the "foo" BCO.

       BCOs contain a list of predeclared identifiers (see localVariables).
       These are the names of the parameters used to invoke the parameters,
       plus the names of predeclared local variables.
       Predeclaring a variable will give it a known address at compile time,
       allowing faster code to be generated.
       The compiler will use this instead of a <code>dimloc</code> instruction
       when it can prove that it's safe to do so. */
    class BytecodeObject : public afl::base::RefCounted {
     public:
        typedef std::vector<Opcode>::size_type PC_t;
        typedef uint16_t Label_t;

        /** Constructor.
            Make blank object. */
        BytecodeObject();

        /** Destructor. */
        ~BytecodeObject();

        /*
         *  Locals
         */

        /** Add named argument.
            addArgument(name, true) should not be followed by addArgument(name, false).
            \param name Name of argument
            \param optional true if this argument is optional */
        void addArgument(String_t name, bool optional);

        /** Add local variable.
            addLocalVariable() should not be followed by addArgument().
            \param name Name
            \return address for new local variable */
        uint16_t addLocalVariable(const String_t& name);

        /** Check whether local variable is present.
            \param name Name to check
            \return true local variable present (addLocalVariable, addArgument) */
        bool hasLocalVariable(const String_t& name);

        /*
         *  Identifying Information
         */

        /** Get the "is procedure" flag.
            \see setIsProcedure
            \return flag */
        bool isProcedure() const;

        /** Set the "is procedure" flag.
            If set, the generated code is a procedure without result, which leaves the stack as-is.
            If clear, the generated code is a function which generates a single result on the stack.
            \param flag Flag */
        void setIsProcedure(bool flag);

        /** Get the "is varargs" flag.
            \see setIsVarargs
            \return flag */
        bool isVarargs() const;

        /** Set the "is varargs" flag.
            If set, arguments exceeding the maximum number of args are wrapped into an array.
            \param flag Flag */
        void setIsVarargs(bool flag);

        /** Get minimum number of arguments.
            \return minimum number of arguments */
        size_t getMinArgs() const;

        /** Set minimum number of arguments.
            For deserialisation use; addArgument() will manage this value automatically.
            \param n minimum number of arguments */
        void setMinArgs(size_t n);

        /** Get maximum number of arguments.
            \return maximum number of arguments */
        size_t   getMaxArgs() const;

        /** Set maximum number of arguments.
            For deserialisation use; addArgument() will manage this value automatically.
            \param n maximum number of arguments */
        void     setMaxArgs(size_t n);

        /** Get subroutine name.
            This is the name of the subroutine as it was originally defined.
            It is used for debugging.
            The current subroutine with that name might already be different.
            \return name */
        String_t getSubroutineName() const;

        /** Set subroutine name.
            \param name Name
            \see getSubroutineName */
        void setSubroutineName(String_t name);

        /** Get origin identifier.
            This name is used for identifying this code; typically, a plugin name.
            It has no internal significance and should be human-readable.
            \return name */
        String_t getOrigin() const;

        /** Set origin identifier.
            \param origin name
            \see getOrigin */
        void setOrigin(const String_t& origin);

        /** Get file name.
            For debugging, the name of the file this code came from.
            It has no internal significance and should be human-readable.
            \return name */
        String_t getFileName() const;

        /** Set file name.
            \param fileName name
            \see setFileName */
        void setFileName(String_t fileName);

        /** Remember current line number.
            Declares that future addInstruction() and friendy correspond to code from the given line number.
            Used for debugging.
            \param line Line number */
        void addLineNumber(uint32_t line);

        /** Add line/address pair.
            For use in deserialisation; do not use for compilation.
            \param line Line number
            \param pc Program counter */
        void addLineNumber(uint32_t line, uint32_t pc);

        /** Get line number for program counter.
            \param pc Program counter
            \return line number (0 if not found) */
        uint32_t getLineNumber(PC_t pc) const;

        /*
         *  Code Generation
         */

        /** Make a new label for future reference.
            This label can be used in as many jumps as needed (addJump),
            and must be placed exactly once using addLabel.
            \return label identifier */
        Label_t  makeLabel();

        /** Add an instruction.
            \param major Major opcode
            \param minor Minor opcode
            \param arg   Argument */
        void addInstruction(Opcode::Major major, uint8_t minor, uint16_t arg);

        /** Add a variable-referencing instruction.
            Selects the optimum minor/arg for referencing the given variable in the current context.
            \param major Major opcode (maPush, maPop, maStore)
            \param name  Name
            \param cc    Compilation context */
        void addVariableReferenceInstruction(Opcode::Major major, const String_t& name, const CompilationContext& cc);

        /** Place a label.
            \param label Label to place (from makeLabel()) */
        void addLabel(Label_t label);

        /** Insert a label in the middle of code.
            Note that this is slow and should be used only infrequently.
            \param label Label to place (from makeLabel())
            \param pc    Insert label before this instruction */
        void insertLabel(Label_t label, PC_t pc);

        /** Add jump instruction.
            \param flags Jump condition (jIfTrue, jPopAlways, etc.)
            \param label Label to place (from makeLabel()) */
        void addJump(uint8_t flags, Label_t label);

        /** Add push-literal instruction.
            Selects the optimum instruction for creating the given literal.
            \param literal Literal to push */
        void addPushLiteral(const afl::data::Value* literal);

        /** Add name (symbol) for later reference.
            Existing names are recycled if possible.
            \param name Name */
        uint16_t addName(String_t name);

        /** Check whether name already referenced.
            \param name Name to check
            \return true if name already referenced */
        bool hasName(String_t name) const;

        /** Check for potential call into user code.
            This may inhibit some optimisations.
            Potential user calls are:
            - all xxxind instructions (potential calls)
            - sevalx, sevals, srunhook (unknown code)
            \return true if user call found */
        bool hasUserCall() const;

        /** Turn symbolic references into absolute references.
            Removes symbolic label instructions and transforms symbolic jumps into absolute.
            Absolute labels (=nops) are also removed.
            If the code is too large, this function silently does nothing;
            code can still be executed, just slower. */
        void relocate();

        /** Compact code.
            Removes absolute labels (=nops).
            This is a subset of relocate() used for optimisation. */
        void compact();

        /** Copy local variables from another BCO.
            \param other Source */
        void copyLocalVariablesFrom(const BytecodeObject& other);

        /** Append code from another BCO.
            Instructions are adjusted to refer to our name/literal tables.
            \param other Source */
        void append(const BytecodeObject& other);

        /*
         *  Access
         */

        /** Get number of instructions.
            \return number of instructions */
        PC_t getNumInstructions() const;

        /** Get number of labels.
            \return number of labels */
        Label_t getNumLabels() const;

        /** Set number of labels.
            For use in deserialisation only.
            \param n Number */
        void setNumLabels(uint16_t n);

        /** Find jump target.
            If the jump is symbolic, looks up the target label.
            \param minor Minor opcode from the maJump instruction
            \param arg Parameter
            \return target program counter (on error, may be larger than getNumInstructions()
            to tell caller to stop executing this BCO) */
        PC_t getJumpTarget(uint8_t minor, uint16_t arg) const;

        /** Access instruction by PC.
            \param index Program counter
            \return instruction */
        Opcode&  operator()(PC_t index);
        const Opcode& operator()(PC_t index) const;

        /** Format instruction in human-readable way.
            \param index Program counter
            \param w     World (used for global variable names)
            \return text */
        String_t getDisassembly(PC_t index, const World& w) const;

        /** Get literal from literal table.
            \param index Index
            \return literal (owned by BytecodeObject) */
        afl::data::Value* getLiteral(uint16_t index) const;

        /** Get name from referenced-name table.
            \param index Index [0,names().getNumNames()) */
        const String_t& getName(uint16_t index) const;

        /** Access local variable names.
            \return names */
        const afl::data::NameMap& localVariables() const;
        afl::data::NameMap& localVariables();

        /** Access referenced names.
            \return names */
        const afl::data::NameMap& names() const;
        afl::data::NameMap& names();

        /** Access literals.
            \return literal table */
        const afl::data::Segment& literals() const;
        afl::data::Segment& literals();

        /** Access code.
            \return code */
        const std::vector<Opcode>& code() const;

        /** Access line number table.
            \see getLineNumber()
            \return line number table; pairs of address/line. */
        const std::vector<uint32_t>& lineNumbers() const;

     private:
        afl::data::Segment    m_literals;  ///< Literals referenced in bytecode.
        afl::data::NameMap    m_names;     ///< Names referenced in bytecode.
        std::vector<Opcode>   m_code;      ///< Actual code.
        Label_t               m_numLabels;
        afl::data::NameMap    m_localVariables;
        size_t                m_minArgs;
        size_t                m_maxArgs;
        bool                  m_isProcedure;
        bool                  m_isVarargs;
        String_t              m_subroutineName;
        String_t              m_fileName;
        String_t              m_origin;
        std::vector<uint32_t> m_lineNumbers; ///< Line numbers. Pairs of address,line.
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

// Add local variable.
inline uint16_t
interpreter::BytecodeObject::addLocalVariable(const String_t& name)
{
    // ex IntBytecodeObject::addLocalVariable
    return uint16_t(m_localVariables.add(name));
}

// Check whether local variable is present.
inline bool
interpreter::BytecodeObject::hasLocalVariable(const String_t& name)
{
    // ex IntBytecodeObject::hasLocalVariable
    return m_localVariables.getIndexByName(name) != m_localVariables.nil;
}

// Get the "is procedure" flag.
inline bool
interpreter::BytecodeObject::isProcedure() const
{
    // ex IntBytecodeObject::isProcedure
    return m_isProcedure;
}

// Set the "is procedure" flag.
inline void
interpreter::BytecodeObject::setIsProcedure(bool flag)
{
    // ex IntBytecodeObject::setIsProcedure
    m_isProcedure = flag;
}

// Get the "is varargs" flag.
inline bool
interpreter::BytecodeObject::isVarargs() const
{
    // ex IntBytecodeObject::isVarargs
    return m_isVarargs;
}

// Set the "is varargs" flag.
inline void
interpreter::BytecodeObject::setIsVarargs(bool flag)
{
    // ex IntBytecodeObject::setIsVarargs
    m_isVarargs = flag;
}

// Get minimum number of arguments.
inline size_t
interpreter::BytecodeObject::getMinArgs() const
{
    // ex IntBytecodeObject::getMinArgs
    return m_minArgs;
}

// Set minimum number of arguments.
inline void
interpreter::BytecodeObject::setMinArgs(size_t n)
{
    m_minArgs = n;
}

// Get maximum number of arguments.
inline size_t
interpreter::BytecodeObject::getMaxArgs() const
{
    // ex IntBytecodeObject::getMaxArgs
    return m_maxArgs;
}

// Set maximum number of arguments.
inline void
interpreter::BytecodeObject::setMaxArgs(size_t n)
{
    m_maxArgs = n;
}

// Check whether name already referenced.
inline bool
interpreter::BytecodeObject::hasName(String_t name) const
{
    // ex IntBytecodeObject::hasName
    return m_names.getIndexByName(name) != m_names.nil;
}

// Get number of instructions.
inline interpreter::BytecodeObject::PC_t
interpreter::BytecodeObject::getNumInstructions() const
{
    // ex IntBytecodeObject::getNumInstructions
    return m_code.size();
}

// Get number of labels.
inline interpreter::BytecodeObject::Label_t
interpreter::BytecodeObject::getNumLabels() const
{
    // ex IntBytecodeObject::getNumLabels
    return m_numLabels;
}

// Set number of labels.
inline void
interpreter::BytecodeObject::setNumLabels(uint16_t n)
{
    m_numLabels = n;
}

// Access instruction by PC.
inline interpreter::Opcode&
interpreter::BytecodeObject::operator()(PC_t index)
{
    return m_code[index];
}

inline const interpreter::Opcode&
interpreter::BytecodeObject::operator()(PC_t index) const
{
    return m_code[index];
}

// Get literal from literal table.
inline afl::data::Value*
interpreter::BytecodeObject::getLiteral(uint16_t index) const
{
    // ex IntBytecodeObject::getLiteral
    // No range check required; it is done by Segment
    return m_literals[index];
}

// Get name from referenced-name table.
inline const String_t&
interpreter::BytecodeObject::getName(uint16_t index) const
{
    // FIXME: deal with out-of-range somehow!
    return m_names.getNameByIndex(index);
}

// Access local variable names.
inline const afl::data::NameMap&
interpreter::BytecodeObject::localVariables() const
{
    // ex IntBytecodeObject::getLocalNames
    return m_localVariables;
}

inline afl::data::NameMap&
interpreter::BytecodeObject::localVariables()
{
    return m_localVariables;
}

// Access names.
inline const afl::data::NameMap&
interpreter::BytecodeObject::names() const
{
    return m_names;
}

inline afl::data::NameMap&
interpreter::BytecodeObject::names()
{
    return m_names;
}

// Access literals.
inline const afl::data::Segment&
interpreter::BytecodeObject::literals() const
{
    return m_literals;
}

inline afl::data::Segment&
interpreter::BytecodeObject::literals()
{
    return m_literals;
}

// Access code.
inline const std::vector<interpreter::Opcode>&
interpreter::BytecodeObject::code() const
{
    return m_code;
}

// Access line number table.
inline const std::vector<uint32_t>&
interpreter::BytecodeObject::lineNumbers() const
{
    return m_lineNumbers;
}

#endif
