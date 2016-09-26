/**
  *  \file interpreter/expr/rvaluenode.cpp
  */

#include "interpreter/expr/rvaluenode.hpp"
#include "interpreter/error.hpp"

void
interpreter::expr::RValueNode::compileStore(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/, Node& /*rhs*/)
{
    // ex IntRValueExprNode::compileStore
    throw Error::notAssignable();
}

void
interpreter::expr::RValueNode::compileRead(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/)
{
    // ex IntRValueExprNode::compileRead
    throw Error::notAssignable();
}

void
interpreter::expr::RValueNode::compileWrite(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/)
{
    // ex IntRValueExprNode::compileWrite
    throw Error::notAssignable();
}
