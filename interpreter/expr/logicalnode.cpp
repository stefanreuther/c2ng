/**
  *  \file interpreter/expr/logicalnode.cpp
  */

#include "interpreter/expr/logicalnode.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/binaryoperation.hpp"

/** Constructor.
    \param shortcut_jump Condition for the shortcut jump
    \param binary_op     Minor opcode for the binary operation */
interpreter::expr::LogicalNode::LogicalNode(uint8_t shortcut_jump, uint8_t binary_op)
    : shortcut_jump(shortcut_jump),
      binary_op(binary_op)
{
    // ex IntLogicalNode::IntLogicalNode
}

void
interpreter::expr::LogicalNode::compileValue(BytecodeObject& bco, const CompilationContext& cc)
{
    // ex IntLogicalNode::compileValue

    // We need the 'ubool' instruction to guarantee the result to be boolean even
    // when we take the shortcut. The only exception is Xor, where the shortcut
    // value is "singular".
    // a And b       a Or b        a Xor b
    // => a          a             a
    //    ubool      ubool        [ubool]
    //    jf fini    jt fini       je fini
    //    b          b             b
    //    band       bor           bxor
    // fini:      fini:          fini:

    BytecodeObject::Label_t fini = bco.makeLabel();

    a->compileValue(bco, cc);
    if (binary_op != biXor) {
        bco.addInstruction(Opcode::maUnary, unBool, 0);
    }
    bco.addJump(shortcut_jump, fini);
    b->compileValue(bco, cc);
    bco.addInstruction(Opcode::maBinary, binary_op, 0);
    bco.addLabel(fini);
}

void
interpreter::expr::LogicalNode::compileEffect(BytecodeObject& bco, const CompilationContext& cc)
{
    defaultCompileEffect(bco, cc);
}

void
interpreter::expr::LogicalNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    // ex IntLogicalNode::compileCondition

    // a And b       a Or b        a Xor b
    // => a          => a          => a
    //    jfep iff      jtp ift       je skip
    //    b             b             b
    //    jfep iff      jtp ift       bxor
    //    j ift         j iff      skip: jtp ift
    //                                j iff

    if (binary_op == biAnd) {
        // FIXME(?): this generates different side-effects from compileValue().
        // compileValue() will evaluate RHS if LHS is Empty (to distinguish Empty and False),
        // whereas this implementation will not. If we wish to keep efficient short-circuit
        // evaluation, it probably makes sense to keep this undefined.
        BytecodeObject::Label_t x = bco.makeLabel();
        a->compileCondition(bco, cc, x, iff);
        bco.addLabel(x);
        b->compileCondition(bco, cc, ift, iff);
    } else if (binary_op == biOr) {
        BytecodeObject::Label_t x = bco.makeLabel();
        a->compileCondition(bco, cc, ift, x);
        bco.addLabel(x);
        b->compileCondition(bco, cc, ift, iff);
    } else if (binary_op == biXor) {
        // This is harder than the others because we cannot use compileCondition here.
        // Jump threading also does not work, because we have no 'jump and pop only if
        // condition true' instruction. Since Xor is comparatively rare, I hope this
        // isn't too much of a problem.
        BytecodeObject::Label_t x = bco.makeLabel();
        a->compileValue(bco, cc);
        bco.addJump(shortcut_jump, x);
        b->compileValue(bco, cc);
        bco.addInstruction(Opcode::maBinary, binary_op, 0);
        bco.addLabel(x);
        bco.addJump(Opcode::jIfTrue | Opcode::jPopAlways, ift);
        bco.addJump(Opcode::jAlways, iff);
    } else {
        defaultCompileCondition(bco, cc, ift, iff);
    }
}
