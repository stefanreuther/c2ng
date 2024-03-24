/**
  *  \file interpreter/expr/unarynode.hpp
  *  \brief Class interpreter::expr::UnaryNode
  */
#ifndef C2NG_INTERPRETER_EXPR_UNARYNODE_HPP
#define C2NG_INTERPRETER_EXPR_UNARYNODE_HPP

#include "interpreter/expr/rvaluenode.hpp"
#include "interpreter/unaryoperation.hpp"

namespace interpreter { namespace expr {

    /** General unary operation.
        Represents an operation of format "<op> arg" that compiles as "arg, <op>". */
    class UnaryNode : public RValueNode {
     public:
        /** Constructor.
            @param op    Opcode
            @param arg   Operand */
        UnaryNode(UnaryOperation op, const Node& arg)
            : m_op(op), m_arg(arg)
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
        bool is(UnaryOperation op) const
            { return m_op == op; }

     private:
        UnaryOperation m_op;
        const Node& m_arg;
    };

} }

#endif
