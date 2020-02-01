/**
  *  \file interpreter/expr/sequencenode.cpp
  */

#include "interpreter/expr/sequencenode.hpp"

void
interpreter::expr::SequenceNode::compileValue(BytecodeObject& bco, const CompilationContext& cc)
{
    // ex IntSequenceNode::compileValue
    // ex ccexpr.pas:op_SEQUENCE
    a->compileEffect(bco, cc);
    b->compileValue(bco, cc);
}

void
interpreter::expr::SequenceNode::compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc)
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::SequenceNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    defaultCompileCondition(bco, cc, ift, iff);
}
