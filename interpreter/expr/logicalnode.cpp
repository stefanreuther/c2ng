/**
  *  \file interpreter/expr/logicalnode.cpp
  *  \brief Class interpreter::expr::LogicalNode
  */

#include "interpreter/expr/logicalnode.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/binaryoperation.hpp"

interpreter::expr::LogicalNode::LogicalNode(uint8_t shortcutJump, BinaryOperation binaryOp, const Node& left, const Node& right)
    : m_shortcutJump(shortcutJump),
      m_binaryOp(binaryOp),
      m_left(left),
      m_right(right)
{
    // ex IntLogicalNode::IntLogicalNode
}

void
interpreter::expr::LogicalNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
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

    m_left.compileValue(bco, cc);
    if (m_binaryOp != biXor) {
        bco.addInstruction(Opcode::maUnary, unBool, 0);
    }
    bco.addJump(m_shortcutJump, fini);
    m_right.compileValue(bco, cc);
    bco.addInstruction(Opcode::maBinary, m_binaryOp, 0);
    bco.addLabel(fini);
}

void
interpreter::expr::LogicalNode::compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
{
    BytecodeObject::Label_t lab = bco.makeLabel();
    compileCondition(bco, cc, lab, lab);
    bco.addLabel(lab);
}

void
interpreter::expr::LogicalNode::compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
{
    // ex IntLogicalNode::compileCondition

    // a And b       a Or b        a Xor b
    // => a          => a          => a
    //    jfep iff      jtp ift       je skip
    //    b             b             b
    //    jfep iff      jtp ift       bxor
    //    j ift         j iff      skip: jtp ift
    //                                j iff

    if (m_binaryOp == biAnd) {
        // FIXME(?): this generates different side-effects from compileValue().
        // compileValue() will evaluate RHS if LHS is Empty (to distinguish Empty and False),
        // whereas this implementation will not. If we wish to keep efficient short-circuit
        // evaluation, it probably makes sense to keep this undefined.
        BytecodeObject::Label_t x = bco.makeLabel();
        m_left.compileCondition(bco, cc, x, iff);
        bco.addLabel(x);
        m_right.compileCondition(bco, cc, ift, iff);
    } else if (m_binaryOp == biOr) {
        BytecodeObject::Label_t x = bco.makeLabel();
        m_left.compileCondition(bco, cc, ift, x);
        bco.addLabel(x);
        m_right.compileCondition(bco, cc, ift, iff);
    } else if (m_binaryOp == biXor) {
        // This is harder than the others because we cannot use compileCondition here.
        // Jump threading also does not work, because we have no 'jump and pop only if
        // condition true' instruction. Since Xor is comparatively rare, I hope this
        // isn't too much of a problem.
        BytecodeObject::Label_t x = bco.makeLabel();
        m_left.compileValue(bco, cc);
        bco.addJump(m_shortcutJump, x);
        m_right.compileValue(bco, cc);
        bco.addInstruction(Opcode::maBinary, m_binaryOp, 0);
        bco.addLabel(x);
        bco.addJump(Opcode::jIfTrue | Opcode::jPopAlways, ift);
        bco.addJump(Opcode::jAlways, iff);
    } else {
        defaultCompileCondition(bco, cc, ift, iff);
    }
}
