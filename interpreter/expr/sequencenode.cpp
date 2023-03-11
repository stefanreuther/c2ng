/**
  *  \file interpreter/expr/sequencenode.cpp
  *  \brief Class interpreter::expr::SequenceNode
  */

#include "interpreter/expr/sequencenode.hpp"

void
interpreter::expr::SequenceNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    // ex IntSequenceNode::compileValue
    // ex ccexpr.pas:op_SEQUENCE
    m_a.compileEffect(bco, cc);
    m_b.compileValue(bco, cc);
}

void
interpreter::expr::SequenceNode::compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc) const
{
    m_a.compileEffect(bco, cc);
    m_b.compileEffect(bco, cc);
}

void
interpreter::expr::SequenceNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    m_a.compileEffect(bco, cc);
    m_b.compileCondition(bco, cc, ift, iff);
}
