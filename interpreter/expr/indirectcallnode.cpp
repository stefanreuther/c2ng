/**
  *  \file interpreter/expr/indirectcallnode.cpp
  *  \brief Class interpreter::expr::IndirectCallNode
  */

#include "interpreter/expr/indirectcallnode.hpp"

interpreter::expr::IndirectCallNode::IndirectCallNode(const Node& func)
    : m_function(func)
{
    // ex IntIndirectCallNode::IntIndirectCallNode
}

void
interpreter::expr::IndirectCallNode::compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::IndirectCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    // PUSHIND nargs    rr:args:R => rr:result
    for (size_t i = 0; i != args.size(); ++i) {
        args[i]->compileValue(bco, cc);
    }
    m_function.compileValue(bco, cc);
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad + Opcode::miIMRefuseProcedures, uint16_t(args.size()));
}

void
interpreter::expr::IndirectCallNode::compileStore(BytecodeObject& bco, const CompilationContext& cc, const Node& rhs) const
{
    // STOREIND nargs   rr:args:val:R => rr:val
    for (size_t i = 0; i != args.size(); ++i) {
        args[i]->compileValue(bco, cc);
    }
    rhs.compileValue(bco, cc);
    m_function.compileValue(bco, cc);
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMStore + Opcode::miIMRefuseProcedures, uint16_t(args.size()));
}

void
interpreter::expr::IndirectCallNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    defaultCompileCondition(bco, cc, ift, iff);
}

void
interpreter::expr::IndirectCallNode::compileRead(BytecodeObject& bco, const CompilationContext& cc) const
{
    // Compute inputs            => ...:args:func
    for (size_t i = 0; i != args.size(); ++i) {
        args[i]->compileValue(bco, cc);
    }
    m_function.compileValue(bco, cc);

    // Duplicate everything      => ...:args:func:args:func
    size_t nwords = args.size()+1;
    for (size_t i = 0; i < nwords; ++i) {
        bco.addInstruction(Opcode::maStack, Opcode::miStackDup, uint16_t(nwords-1));
    }

    // Read                      => ...:args:func:value
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad + Opcode::miIMRefuseProcedures, uint16_t(args.size()));
}

void
interpreter::expr::IndirectCallNode::compileWrite(BytecodeObject& bco, const CompilationContext& /*cc*/) const
{
    // We have ...:args:func:value,
    // we need ...:args:value:func
    bco.addInstruction(Opcode::maStack, Opcode::miStackSwap, 1);
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMStore + Opcode::miIMRefuseProcedures, uint16_t(args.size()));
}
