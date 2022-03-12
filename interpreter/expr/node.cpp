/**
  *  \file interpreter/expr/node.cpp
  *  \brief Base class interpreter::expr::Node
  */

#include "interpreter/expr/node.hpp"

void
interpreter::expr::Node::defaultCompileEffect(BytecodeObject& bco, const CompilationContext& cc) const
{
    // ex IntExprNode::compileEffect
    // Compute value and discard it
    compileValue(bco, cc);
    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
}

void
interpreter::expr::Node::defaultCompileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    // ex IntExprNode::compileCondition
    // Generate two-way jump
    compileValue(bco, cc);
    bco.addJump(Opcode::jIfTrue | Opcode::jPopAlways, ift);
    bco.addJump(Opcode::jAlways, iff);
}
