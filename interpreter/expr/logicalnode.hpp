/**
  *  \file interpreter/expr/logicalnode.hpp
  *  \brief Class interpreter::expr::LogicalNode
  */
#ifndef C2NG_INTERPRETER_EXPR_LOGICALNODE_HPP
#define C2NG_INTERPRETER_EXPR_LOGICALNODE_HPP

#include "interpreter/binaryoperation.hpp"
#include "interpreter/expr/rvaluenode.hpp"

namespace interpreter { namespace expr {

    /** Logical operator (And/Or/Xor). Generates code for a logical operation using
        short-circuit evaluation. A short-cut jump determines whether the result
        is already known after evaluating the first operand; if both operands have
        to be evaluated, a binary operation combines them into the real result. */
    class LogicalNode : public RValueNode {
     public:
        /** Constructor.
            @param shortcutJump  Minor opcode for shortcut jump (jIfFalse, etc.)
            @param binaryOp      Binary operation (biAnd, etc.)
            @param left          Left (first) operand
            @param right         Right (second) operand */
        LogicalNode(uint8_t shortcutJump, BinaryOperation binaryOp, const Node& left, const Node& right);

        // Node:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;

     private:
        uint8_t m_shortcutJump;
        BinaryOperation m_binaryOp : 8;
        const Node& m_left;
        const Node& m_right;
    };

} }

#endif
