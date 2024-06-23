/**
  *  \file interpreter/expr/membernode.cpp
  *  \brief Class interpreter::expr::MemberNode
  */

#include "interpreter/expr/membernode.hpp"

interpreter::expr::MemberNode::MemberNode(String_t name, const Node& expr)
    : m_name(name), m_expr(expr)
{
    // ex IntMemberNode::IntMemberNode
}

interpreter::expr::MemberNode::~MemberNode()
{
    // ex IntMemberNode::~IntMemberNode
}

void
interpreter::expr::MemberNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    // PUSHMEM nn      rr:R => rr:R.name[nn]
    m_expr.compileValue(bco, cc);
    bco.addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco.addName(m_name));
}

void
interpreter::expr::MemberNode::compileStore(BytecodeObject& bco, const CompilationContext& cc, const Node& rhs) const
{
    // STOREMEM nn     rr:val:R => rr:val
    // 'val' already on stack, so we must compute 'R'
    rhs.compileValue(bco, cc);
    m_expr.compileValue(bco, cc);
    bco.addInstruction(Opcode::maMemref, Opcode::miIMStore, bco.addName(m_name));
}

void
interpreter::expr::MemberNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    defaultCompileCondition(bco, cc, ift, iff);
}

void
interpreter::expr::MemberNode::compileRead(BytecodeObject& bco, const CompilationContext& cc) const
{
    m_expr.compileValue(bco, cc);
    bco.addInstruction(Opcode::maStack,  Opcode::miStackDup, 0);
    bco.addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco.addName(m_name));
}

void
interpreter::expr::MemberNode::compileWrite(BytecodeObject& bco, const CompilationContext& /*cc*/) const
{
    bco.addInstruction(Opcode::maStack,  Opcode::miStackSwap, 1);
    bco.addInstruction(Opcode::maMemref, Opcode::miIMStore, bco.addName(m_name));
}
