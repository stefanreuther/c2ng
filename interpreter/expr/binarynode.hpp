/**
  *  \file interpreter/expr/binarynode.hpp
  *  \brief Class interpreter::expr::BinaryNode
  */
#ifndef C2NG_INTERPRETER_EXPR_BINARYNODE_HPP
#define C2NG_INTERPRETER_EXPR_BINARYNODE_HPP

#include "interpreter/expr/rvaluenode.hpp"
#include "interpreter/binaryoperation.hpp"

namespace interpreter { namespace expr {

    /** General binary operation.
        Represents an operation of format "left <op> right" that compiles as "left value, right value, <op>". */
    class BinaryNode : public RValueNode {
     public:
        /** Constructor.
            @param op    Opcode
            @param left  Left (first) operand
            @param right Right (second) operand */
        BinaryNode(BinaryOperation op, const Node& left, const Node& right)
            : m_op(op), m_left(left), m_right(right)
            {
                // ex IntSimpleExprNode::IntSimpleExprNode (part)
            }

        // Node:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;

        /** Check for specific operation.
            @param op   Opcode
            @return true when matching the given opcode */
        bool is(BinaryOperation op) const
            { return m_op == op; }

     private:
        BinaryOperation m_op;
        const Node& m_left;
        const Node& m_right;
    };

} }

#endif
