/**
  *  \file interpreter/expr/identifiernode.cpp
  */

#include "interpreter/expr/identifiernode.hpp"

/** Constructor.
    \param name Name of identifier */
interpreter::expr::IdentifierNode::IdentifierNode(String_t name)
    : name(name)
{
    // ex IntIdentifierNode::IntIdentifierNode
}

interpreter::expr::IdentifierNode::~IdentifierNode()
{
    // ex IntIdentifierNode::~IntIdentifierNode
}

void
interpreter::expr::IdentifierNode::compileEffect(BytecodeObject& bco, const CompilationContext& cc)
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::IdentifierNode::compileValue(BytecodeObject& bco, const CompilationContext& cc)
{
    // ex IntIdentifierNode::compileValue
    bco.addVariableReferenceInstruction(Opcode::maPush, name, cc);
}

void
interpreter::expr::IdentifierNode::compileStore(BytecodeObject& bco, const CompilationContext& cc, Node& rhs)
{
    rhs.compileValue(bco, cc);
    bco.addVariableReferenceInstruction(Opcode::maStore, name, cc);
}

void
interpreter::expr::IdentifierNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    defaultCompileCondition(bco, cc, ift, iff);
}

void
interpreter::expr::IdentifierNode::compileRead(BytecodeObject& bco, const CompilationContext& cc)
{
    bco.addVariableReferenceInstruction(Opcode::maPush, name, cc);
}

void
interpreter::expr::IdentifierNode::compileWrite(BytecodeObject& bco, const CompilationContext& cc)
{
    bco.addVariableReferenceInstruction(Opcode::maStore, name, cc);
}
