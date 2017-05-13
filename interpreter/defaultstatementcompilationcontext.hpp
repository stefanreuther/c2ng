/**
  *  \file interpreter/defaultstatementcompilationcontext.hpp
  *  \brief Class interpreter::DefaultStatementCompilationContext
  */
#ifndef C2NG_INTERPRETER_DEFAULTSTATEMENTCOMPILATIONCONTEXT_HPP
#define C2NG_INTERPRETER_DEFAULTSTATEMENTCOMPILATIONCONTEXT_HPP

#include "interpreter/statementcompilationcontext.hpp"

namespace interpreter {

    /** StatementCompilationContext implementation with default behaviour.
        This implementation relays all methods to their default behaviour, that is,
        use the parent SCC's methods or fail with an error/ignore the call.

        Use DefaultStatementCompilationContext if you need a new StatementCompilationContext to be able to pass different flags,
        but not change the behaviour. */
    class DefaultStatementCompilationContext : public StatementCompilationContext {
     public:
        /** Constructor, use as root SCC.
            This StatementCompilationContext will not have a parent context and thus fail compileBreak/compileContinue and ignore compileCleanup.
            \param world World to live in */
        explicit DefaultStatementCompilationContext(World& world)
            : StatementCompilationContext(world)
            { }

        /** Constructor, use parent SCC.
            This StatementCompilationContext will relay its methods to the parent's methods.
            \param parent Parent SCC */
        explicit DefaultStatementCompilationContext(const StatementCompilationContext& parent)
            : StatementCompilationContext(parent)
            { }

        // StatementCompilationContext:
        virtual void compileBreak(BytecodeObject& bco) const;
        virtual void compileContinue(BytecodeObject& bco) const;
        virtual void compileCleanup(BytecodeObject& bco) const;

    };

}

#endif
