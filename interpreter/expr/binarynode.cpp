/**
  *  \file interpreter/expr/binarynode.cpp
  *  \brief Class interpreter::expr::BinaryNode
  */

#include "interpreter/expr/binarynode.hpp"

void
interpreter::expr::BinaryNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    m_left.compileValue(bco, cc);
    m_right.compileValue(bco, cc);
    bco.addInstruction(Opcode::maBinary, m_op, 0);
}

void
interpreter::expr::BinaryNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    defaultCompileCondition(bco, cc, ift, iff);
}
