/**
  *  \file interpreter/expr/casenode.hpp
  *  \brief Class interpreter::expr::CaseNode
  */
#ifndef C2NG_INTERPRETER_EXPR_CASENODE_HPP
#define C2NG_INTERPRETER_EXPR_CASENODE_HPP

#include "afl/base/deleter.hpp"
#include "interpreter/expr/rvaluenode.hpp"

namespace interpreter { namespace expr {

    /** Case-sensitive expression node.
        Represents a binary operation that is affected by StrCase(). */
    class CaseNode : public RValueNode {
     public:
        /** Constructor.
            @param minor   Minor opcode (used for case-sensitive mode, +1 for case-insensitive)
            @param left    Left operand
            @param right   Right operand */
        CaseNode(uint8_t minor, const Node& left, const Node& right);

        // Node:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;

        /** Try to convert to assignment.
            If this is an equality comparison, creates a new AssignmentNode containing our child nodes, and returns that.
            Otherwise, just returns *this.

            This is used to convert a top-level "a = b" statement into an assignment.

            @param del Deleter to hold potentially created new nodes */
        const Node& convertToAssignment(afl::base::Deleter& del) const;

     private:
        uint8_t m_minor;
        const Node& m_left;
        const Node& m_right;
    };

} }

#endif
