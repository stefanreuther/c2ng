/**
  *  \file interpreter/expr/assignmentnode.cpp
  */

#include "interpreter/expr/assignmentnode.hpp"

void
interpreter::expr::AssignmentNode::compileValue(BytecodeObject& bco, const CompilationContext& cc)
{
    // ex IntAssignmentNode::compileValue
    // ex ccexpr.pas:op_ASSIGN2 (sort-of)
    a->compileStore(bco, cc, *b);
}

void
interpreter::expr::AssignmentNode::compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc)
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::AssignmentNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    defaultCompileCondition(bco, cc, ift, iff);
}

