/**
  *  \file interpreter/expr/sequencenode.hpp
  *  \brief Class interpreter::expr::SequenceNode
  */
#ifndef C2NG_INTERPRETER_EXPR_SEQUENCENODE_HPP
#define C2NG_INTERPRETER_EXPR_SEQUENCENODE_HPP

#include "interpreter/expr/rvaluenode.hpp"

namespace interpreter { namespace expr {

    /** Sequence node. Evaluates a, then b. */
    class SequenceNode : public RValueNode {
     public:
        /** Constructor.
            @param a First expression
            @param b Second expression */
        SequenceNode(const Node& a, const Node& b)
            : m_a(a), m_b(b)
            { }
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;
     private:
        const Node& m_a;
        const Node& m_b;
    };

} }

#endif
