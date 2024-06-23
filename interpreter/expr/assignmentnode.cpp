/**
  *  \file interpreter/expr/assignmentnode.cpp
  *  \brief Class interpreter::expr::AssignmentNode
  */

#include "interpreter/expr/assignmentnode.hpp"

void
interpreter::expr::AssignmentNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    // ex IntAssignmentNode::compileValue
    // ex ccexpr.pas:op_ASSIGN2 (sort-of)
    m_a.compileStore(bco, cc, m_b);
}

void
interpreter::expr::AssignmentNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    defaultCompileCondition(bco, cc, ift, iff);
}
