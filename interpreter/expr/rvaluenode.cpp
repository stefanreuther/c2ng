/**
  *  \file interpreter/expr/rvaluenode.cpp
  *  \brief Class interpreter::expr::RValueNode
  */

#include "interpreter/expr/rvaluenode.hpp"
#include "interpreter/error.hpp"

void
interpreter::expr::RValueNode::compileStore(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/, const Node& /*rhs*/) const
{
    // ex IntRValueExprNode::compileStore
    throw Error::notAssignable();
}

void
interpreter::expr::RValueNode::compileRead(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
{
    // ex IntRValueExprNode::compileRead
    throw Error::notAssignable();
}

void
interpreter::expr::RValueNode::compileWrite(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
{
    // ex IntRValueExprNode::compileWrite
    throw Error::notAssignable();
}
