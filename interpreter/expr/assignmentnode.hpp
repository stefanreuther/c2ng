/**
  *  \file interpreter/expr/assignmentnode.hpp
  *  \brief Class interpreter::expr::AssignmentNode
  */
#ifndef C2NG_INTERPRETER_EXPR_ASSIGNMENTNODE_HPP
#define C2NG_INTERPRETER_EXPR_ASSIGNMENTNODE_HPP

#include "interpreter/expr/rvaluenode.hpp"

namespace interpreter { namespace expr {

    /** Assignment node.
        Represents an operation of the form "a := b". */
    class AssignmentNode : public RValueNode {
     public:
        /** Constructor.
            @param a Left side (assignment target)
            @param b Right side (new value */
        AssignmentNode(const Node& a, const Node& b)
            : m_a(a), m_b(b)
            { }

        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;

     private:
        const Node& m_a;
        const Node& m_b;
    };

} }

#endif
