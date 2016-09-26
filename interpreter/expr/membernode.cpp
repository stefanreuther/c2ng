/**
  *  \file interpreter/expr/membernode.cpp
  */

#include "interpreter/expr/membernode.hpp"


/** Constructor.
    \param name Name of member */
interpreter::expr::MemberNode::MemberNode(String_t name)
    : m_name(name), m_expr()
{
    // ex IntMemberNode::IntMemberNode
}

interpreter::expr::MemberNode::~MemberNode()
{
    // ex IntMemberNode::~IntMemberNode
}

void
interpreter::expr::MemberNode::setNewExpression(Node* expr) throw()
{
    // ex IntMemberNode::setExpression
    m_expr.reset(expr);
}

void
interpreter::expr::MemberNode::compileEffect(BytecodeObject& bco, const CompilationContext& cc)
{
    defaultCompileEffect(bco, cc);
}                                             

void
interpreter::expr::MemberNode::compileValue(BytecodeObject& bco, const CompilationContext& cc)
{
    // PUSHMEM nn      rr:R => rr:R.name[nn]
    m_expr->compileValue(bco, cc);
    bco.addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco.addName(m_name));
}

void
interpreter::expr::MemberNode::compileStore(BytecodeObject& bco, const CompilationContext& cc, Node& rhs)
{
    // STOREMEM nn     rr:val:R => rr:val
    // 'val' already on stack, so we must compute 'R'
    rhs.compileValue(bco, cc);
    m_expr->compileValue(bco, cc);
    bco.addInstruction(Opcode::maMemref, Opcode::miIMStore, bco.addName(m_name));
}

void
interpreter::expr::MemberNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    defaultCompileCondition(bco, cc, ift, iff);
}

void
interpreter::expr::MemberNode::compileRead(BytecodeObject& bco, const CompilationContext& cc)
{
    m_expr->compileValue(bco, cc);
    bco.addInstruction(Opcode::maStack,  Opcode::miStackDup, 0);
    bco.addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco.addName(m_name));
}

void
interpreter::expr::MemberNode::compileWrite(BytecodeObject& bco, const CompilationContext& /*cc*/)
{
    bco.addInstruction(Opcode::maStack,  Opcode::miStackSwap, 1);
    bco.addInstruction(Opcode::maMemref, Opcode::miIMStore, bco.addName(m_name));
}
