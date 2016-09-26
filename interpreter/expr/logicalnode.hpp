/**
  *  \file interpreter/expr/logicalnode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_LOGICALNODE_HPP
#define C2NG_INTERPRETER_EXPR_LOGICALNODE_HPP

#include "interpreter/expr/simplervaluenode.hpp"

namespace interpreter { namespace expr {

    /** Logical operator (And/Or/Xor). Generates code for a logical operation using
        short-circuit evaluation. A short-cut jump determines whether the result
        is already known after evaluating the first operand; if both operands have
        to be evaluated, a binary operation combines them into the real result. */
    class LogicalNode : public SimpleRValueNode {
     public:
        LogicalNode(uint8_t shortcut_jump, uint8_t binary_op);
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);

     private:
        uint8_t shortcut_jump;
        uint8_t binary_op;
    };

} }

#endif
