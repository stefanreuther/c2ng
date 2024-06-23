/**
  *  \file interpreter/expr/identifiernode.cpp
  *  \brief Class interpreter::expr::IdentifierNode
  */

#include "interpreter/expr/identifiernode.hpp"

interpreter::expr::IdentifierNode::IdentifierNode(String_t name)
    : m_name(name)
{
    // ex IntIdentifierNode::IntIdentifierNode
}

interpreter::expr::IdentifierNode::~IdentifierNode()
{
    // ex IntIdentifierNode::~IntIdentifierNode
}

void
interpreter::expr::IdentifierNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    // ex IntIdentifierNode::compileValue
    bco.addVariableReferenceInstruction(Opcode::maPush, m_name, cc);
}

void
interpreter::expr::IdentifierNode::compileStore(BytecodeObject& bco, const CompilationContext& cc, const Node& rhs) const
{
    rhs.compileValue(bco, cc);
    bco.addVariableReferenceInstruction(Opcode::maStore, m_name, cc);
}

void
interpreter::expr::IdentifierNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    defaultCompileCondition(bco, cc, ift, iff);
}

void
interpreter::expr::IdentifierNode::compileRead(BytecodeObject& bco, const CompilationContext& cc) const
{
    bco.addVariableReferenceInstruction(Opcode::maPush, m_name, cc);
}

void
interpreter::expr::IdentifierNode::compileWrite(BytecodeObject& bco, const CompilationContext& cc) const
{
    bco.addVariableReferenceInstruction(Opcode::maStore, m_name, cc);
}
