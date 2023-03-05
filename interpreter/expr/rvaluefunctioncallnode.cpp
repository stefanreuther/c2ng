/**
  *  \file interpreter/expr/rvaluefunctioncallnode.cpp
  *  \brief Class interpreter::expr::RValueFunctionCallNode
  */

#include "interpreter/expr/rvaluefunctioncallnode.hpp"
#include "interpreter/error.hpp"

void
interpreter::expr::RValueFunctionCallNode::compileStore(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/, const Node& /*rhs*/) const
{
    // ex IntRValueExprNode::compileStore
    throw Error::notAssignable();
}

void
interpreter::expr::RValueFunctionCallNode::compileRead(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
{
    // ex IntRValueExprNode::compileRead
    throw Error::notAssignable();
}

void
interpreter::expr::RValueFunctionCallNode::compileWrite(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
{
    // ex IntRValueExprNode::compileWrite
    throw Error::notAssignable();
}
