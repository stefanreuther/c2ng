/**
  *  \file interpreter/expr/membernode.hpp
  *  \brief Class interpreter::expr::MemberNode
  */
#ifndef C2NG_INTERPRETER_EXPR_MEMBERNODE_HPP
#define C2NG_INTERPRETER_EXPR_MEMBERNODE_HPP

#include "interpreter/expr/node.hpp"

namespace interpreter { namespace expr {

    /** Member access. Implements "someexpr.member". */
    class MemberNode : public Node {
     public:
        /** Constructor.
            @param name  Member name
            @param expr  Object expression */
        MemberNode(String_t name, const Node& expr);

        /** Destructor. */
        ~MemberNode();

        // Node:
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileStore(BytecodeObject& bco, const CompilationContext& cc, const Node& rhs) const;
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;
        void compileRead(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileWrite(BytecodeObject& bco, const CompilationContext& cc) const;

     private:
        String_t m_name;
        const Node& m_expr;
    };

} }

#endif
