/**
  *  \file interpreter/expr/simplenode.cpp
  */

#include "interpreter/expr/simplenode.hpp"

interpreter::expr::SimpleNode::SimpleNode(Opcode::Major major, uint8_t minor)
    : major(major), minor(minor)
{
    // ex IntSimpleExprNode::IntSimpleExprNode
}

void
interpreter::expr::SimpleNode::compileValue(BytecodeObject& bco, const CompilationContext& cc)
{
    // ex IntSimpleExprNode::compileValue
    if (a) {
        a->compileValue(bco, cc);
    }
    if (b) {
        b->compileValue(bco, cc);
    }
    if (c) {
        c->compileValue(bco, cc);
    }
    bco.addInstruction(major, minor, 0);
}

void
interpreter::expr::SimpleNode::compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc)
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::SimpleNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    defaultCompileCondition(bco, cc, ift, iff);
}
