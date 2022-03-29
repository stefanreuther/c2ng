/**
  *  \file interpreter/statementcompilationcontext.hpp
  *  \brief Class interpreter::StatementCompilationContext
  */
#ifndef C2NG_INTERPRETER_STATEMENTCOMPILATIONCONTEXT_HPP
#define C2NG_INTERPRETER_STATEMENTCOMPILATIONCONTEXT_HPP

#include "interpreter/compilationcontext.hpp"

namespace interpreter {

    class StaticContext;
    class BytecodeObject;
    class World;

    /** Statement compilation context.
        In addition to some flags, this also manages an execution context and code generation for Break/Continue/Return.

        Despite having virtual functions, this class has no virtual destructor.
        Objects of its type are not allocated dynamically. */
    class StatementCompilationContext : public CompilationContext {
     public:
        /** Constructor. */
        StatementCompilationContext(World& world);

        /** Construct child context.
            \param parent Parent. We inherit all properties from it. */
        StatementCompilationContext(const StatementCompilationContext& parent);

        virtual void compileBreak(BytecodeObject& bco) const = 0;
        virtual void compileContinue(BytecodeObject& bco) const = 0;
        virtual void compileCleanup(BytecodeObject& bco) const = 0;

        /** Add a flag.
            \param flag flag to add
            \return *this */
        StatementCompilationContext& withFlag(Flag flag);

        /** Remove a flag.
            \param flag flag to remove
            \return *this */
        StatementCompilationContext& withoutFlag(Flag flag);

        /** Set static context.
            The static context is used to resolve ambiguous statements.
            It is set to a non-null value if and only if the compiled statement is a one-line statement going to be executed in that context,
            where the context does not yet contain a frame for the BCO we're compiling into.

            If the static context is not set, some statements (see compileAmbiguousStatement) must be compiled to less
            efficient code that determines the context at run-time (originally "evals", now improved).

            Rationale: it must not be set if the script can change the context, which is the case when we have anything that can follow a "Sub" or "Dim".
            Hence, only one-liners which cannot have anything that follows.
            It must be set, however, when executing the one-liner that results from the "evals" instruction, so we can guarantee termination.

            \return *this */
        StatementCompilationContext& withStaticContext(StaticContext* sc);

        /** Set flags for one-line statement syntax.
            - Add RefuseBlocks because we're a one-liner
            - Add ExpressionsAreStatements because we're a substatement
            - Remove WantTerminators to reject code like "If a then EndSub"
            \return *this*/
        StatementCompilationContext& setOneLineSyntax();

        /** Set flags for block statement syntax.
            - Remove RefuseBlocks because we're a block (should already be clear)
            - Add ExpressionsAreStatements because we're a substatement
            - Add WantTerminators to find end of block"
            \return *this */
        StatementCompilationContext& setBlockSyntax();

        /** Get context provider.
            \see withStaticContext
            \return context provider */
        StaticContext* getStaticContext() const;

     protected:
        /** Generate code for "Break" statement.
            This default implementation delegates this to the parent, or fails if we don't have one. */
        void defaultCompileBreak(BytecodeObject& bco) const;

        /** Generate code for "Continue" statement.
            This default implementation delegates this to the parent, or fails if we don't have one. */
        void defaultCompileContinue(BytecodeObject& bco) const;

        /** Generate code to clean up the stack for a "Return" statement.
            This default implementation delegates this to the parent, or does nothing if we don't have one.
            Since exiting a frame cleans up its exception and context, we only need to clean up stack temporaries here. */
        void defaultCompileCleanup(BytecodeObject& bco) const;

     private:
        /** Parent command compilation context.
            Command compilation contexts are chained when nested blocks are used. */
        const StatementCompilationContext* m_parent;

        /** Static context.
            When this is set, keywords are resolved using its context.
            When this is not set, we generate generic code to resolve the ambiguity at runtime. */
        StaticContext* m_staticContext;
    };

}

#endif
