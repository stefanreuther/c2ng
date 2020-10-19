/**
  *  \file interpreter/expr/casenode.cpp
  */

#include "interpreter/expr/casenode.hpp"
#include "interpreter/expr/assignmentnode.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/binaryoperation.hpp"

/** Constructor.
    \param minor Minor opcode for case-sensitive operation */
interpreter::expr::CaseNode::CaseNode(uint8_t minor)
    : minor(minor)
{ }

void
interpreter::expr::CaseNode::compileValue(BytecodeObject& bco, const CompilationContext& cc)
{
    // ex IntCaseExprNode::compileValue
    a->compileValue(bco, cc);
    b->compileValue(bco, cc);
    bco.addInstruction(Opcode::maBinary, cc.hasFlag(CompilationContext::CaseBlind) ? uint8_t(minor+1) : minor, 0);
}

void
interpreter::expr::CaseNode::compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc)
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::CaseNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    defaultCompileCondition(bco, cc, ift, iff);
}

/** Convert to assignment. If this is an equality comparison, creates a new IntAssignmentNode
    containing our child nodes, and returns that. This is a destructive operation which will
    make this IntCaseExprNode unusable for compilation. You are expected to delete it and
    replace the pointer by the new element returned from this function.

    If this is not an equality comparison, does nothing but return 0. */
interpreter::expr::Node*
interpreter::expr::CaseNode::convertToAssignment()
{
    // ex IntCaseExprNode::convertToAssignment()
    if (minor == biCompareEQ) {
        SimpleRValueNode* result = new AssignmentNode();
        result->setBinary(a, b);
        a = b = 0;
        return result;
    } else {
        return 0;
    }
}
