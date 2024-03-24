/**
  *  \file interpreter/expr/unarynode.cpp
  *  \brief Class interpreter::expr::UnaryNode
  */

#include "interpreter/expr/unarynode.hpp"

void
interpreter::expr::UnaryNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    m_arg.compileValue(bco, cc);
    bco.addInstruction(Opcode::maUnary, m_op, 0);
}

void
interpreter::expr::UnaryNode::compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::UnaryNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    defaultCompileCondition(bco, cc, ift, iff);
}
