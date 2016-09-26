/**
  *  \file interpreter/defaultstatementcompilationcontext.hpp
  */
#ifndef C2NG_INTERPRETER_DEFAULTSTATEMENTCOMPILATIONCONTEXT_HPP
#define C2NG_INTERPRETER_DEFAULTSTATEMENTCOMPILATIONCONTEXT_HPP

#include "interpreter/statementcompilationcontext.hpp"

namespace interpreter {

    class DefaultStatementCompilationContext : public StatementCompilationContext {
     public:
        DefaultStatementCompilationContext(World& world)
            : StatementCompilationContext(world)
            { }
        DefaultStatementCompilationContext(const StatementCompilationContext& parent)
            : StatementCompilationContext(parent)
            { }

        virtual void compileBreak(BytecodeObject& bco) const;
        virtual void compileContinue(BytecodeObject& bco) const;
        virtual void compileCleanup(BytecodeObject& bco) const;

    };

}

#endif
