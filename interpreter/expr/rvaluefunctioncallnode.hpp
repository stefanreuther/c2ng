/**
  *  \file interpreter/expr/rvaluefunctioncallnode.hpp
  *  \brief Class interpreter::expr::RValueFunctionCallNode
  */
#ifndef C2NG_INTERPRETER_EXPR_RVALUEFUNCTIONCALLNODE_HPP
#define C2NG_INTERPRETER_EXPR_RVALUEFUNCTIONCALLNODE_HPP

#include "interpreter/expr/functioncallnode.hpp"

namespace interpreter { namespace expr {

    /** R-value function call expression.
        Implements a "fcn(args...)" expression that cannot be assigned-to. */
    class RValueFunctionCallNode : public FunctionCallNode {
     public:
        virtual void compileStore(BytecodeObject& bco, const CompilationContext& cc, const Node& rhs) const;
        virtual void compileRead(BytecodeObject& bco, const CompilationContext& cc) const;
        virtual void compileWrite(BytecodeObject& bco, const CompilationContext& cc) const;
    };

} }

#endif
