/**
  *  \file interpreter/expr/conditionalnode.cpp
  */

#include "interpreter/expr/conditionalnode.hpp"

void
interpreter::expr::ConditionalNode::compileValue(BytecodeObject& bco, const CompilationContext& cc)
{
    // ex IntConditionalNode::compileValue
    BytecodeObject::Label_t ift = bco.makeLabel();
    BytecodeObject::Label_t iff = bco.makeLabel();
    BytecodeObject::Label_t end = bco.makeLabel();

    a->compileCondition(bco, cc, ift, iff);
    bco.addLabel(ift);
    b->compileValue(bco, cc);
    bco.addJump(Opcode::jAlways, end);
    bco.addLabel(iff);
    if (c != 0) {
        c->compileValue(bco, cc);
    } else {
        bco.addPushLiteral(0);
    }
    bco.addLabel(end);
}

void
interpreter::expr::ConditionalNode::compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc)
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::ConditionalNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    defaultCompileCondition(bco, cc, ift, iff);
}
