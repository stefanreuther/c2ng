/**
  *  \file interpreter/expr/casenode.cpp
  *  \brief Class interpreter::expr::CaseNode
  */

#include "interpreter/expr/casenode.hpp"
#include "interpreter/expr/assignmentnode.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/binaryoperation.hpp"

interpreter::expr::CaseNode::CaseNode(uint8_t minor, const Node& left, const Node& right)
    : minor(minor),
      m_left(left),
      m_right(right)
{ }

void
interpreter::expr::CaseNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    // ex IntCaseExprNode::compileValue
    m_left.compileValue(bco, cc);
    m_right.compileValue(bco, cc);
    bco.addInstruction(Opcode::maBinary, cc.hasFlag(CompilationContext::CaseBlind) ? uint8_t(minor+1) : minor, 0);
}

void
interpreter::expr::CaseNode::compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc) const
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::CaseNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    defaultCompileCondition(bco, cc, ift, iff);
}

const interpreter::expr::Node&
interpreter::expr::CaseNode::convertToAssignment(afl::base::Deleter& del) const
{
    // ex IntCaseExprNode::convertToAssignment()
    if (minor == biCompareEQ) {
        return del.addNew(new AssignmentNode(m_left, m_right));
    } else {
        return *this;
    }
}
