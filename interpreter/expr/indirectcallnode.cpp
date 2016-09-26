/**
  *  \file interpreter/expr/indirectcallnode.cpp
  */

#include "interpreter/expr/indirectcallnode.hpp"

interpreter::expr::IndirectCallNode::IndirectCallNode()
    : func(0)
{
    // ex IntIndirectCallNode::IntIndirectCallNode
}

interpreter::expr::IndirectCallNode::~IndirectCallNode()
{
    delete func;
}

/** Set function. IntIndirectCallNode takes ownership. */
void
interpreter::expr::IndirectCallNode::setNewFunction(Node* func)
{
    // ex IntIndirectCallNode::setFunction
    this->func = func;
}

void
interpreter::expr::IndirectCallNode::compileEffect(BytecodeObject& bco, const CompilationContext& cc)
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::IndirectCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc)
{
    // PUSHIND nargs    rr:args:R => rr:result
    for (size_t i = 0; i != args.size(); ++i) {
        args[i]->compileValue(bco, cc);
    }
    func->compileValue(bco, cc);
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad + Opcode::miIMRefuseProcedures, args.size());
}

void
interpreter::expr::IndirectCallNode::compileStore(BytecodeObject& bco, const CompilationContext& cc, Node& rhs)
{
    // STOREIND nargs   rr:args:val:R => rr:val
    for (size_t i = 0; i != args.size(); ++i) {
        args[i]->compileValue(bco, cc);
    }
    rhs.compileValue(bco, cc);
    func->compileValue(bco, cc);
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMStore + Opcode::miIMRefuseProcedures, args.size());
}

void
interpreter::expr::IndirectCallNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    defaultCompileCondition(bco, cc, ift, iff);
}

void
interpreter::expr::IndirectCallNode::compileRead(BytecodeObject& bco, const CompilationContext& cc)
{
    // Compute inputs            => ...:args:func
    for (size_t i = 0; i != args.size(); ++i) {
        args[i]->compileValue(bco, cc);
    }
    func->compileValue(bco, cc);

    // Duplicate everything      => ...:args:func:args:func
    uint32_t nwords = args.size()+1;
    for (uint32_t i = 0; i < nwords; ++i) {
        bco.addInstruction(Opcode::maStack, Opcode::miStackDup, nwords-1);
    }

    // Read                      => ...:args:func:value
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad + Opcode::miIMRefuseProcedures, args.size());
}

void
interpreter::expr::IndirectCallNode::compileWrite(BytecodeObject& bco, const CompilationContext& /*cc*/)
{
    // We have ...:args:func:value,
    // we need ...:args:value:func
    bco.addInstruction(Opcode::maStack, Opcode::miStackSwap, 1);
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMStore + Opcode::miIMRefuseProcedures, args.size());
}
