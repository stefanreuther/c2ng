/**
  *  \file interpreter/expr/node.cpp
  *  \brief Base class interpreter::expr::Node
  */

#include "interpreter/expr/node.hpp"

void
interpreter::expr::Node::compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
{
    // ex IntExprNode::compileEffect
    // Compile to conditional jump.
    // The alternative is to do compileValue() + miStackDrop.
    // That would generate equivalent code after optimisation for most expressions,
    // but less efficient efficient code for LogicalNode.
    BytecodeObject::Label_t lbl = bco.makeLabel();
    compileCondition(bco, cc, lbl, lbl);
    bco.addLabel(lbl);
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
