/**
  *  \file interpreter/expr/membernode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_MEMBERNODE_HPP
#define C2NG_INTERPRETER_EXPR_MEMBERNODE_HPP

#include <memory>
#include "interpreter/expr/node.hpp"

namespace interpreter { namespace expr {

    /** Member access. Implements "someexpr.member". */
    class MemberNode : public Node {
     public:
        MemberNode(String_t name);
        ~MemberNode();
        void setNewExpression(Node* expr) throw();
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc);
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileStore(BytecodeObject& bco, const CompilationContext& cc, Node& rhs);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
        void compileRead(BytecodeObject& bco, const CompilationContext& cc);
        void compileWrite(BytecodeObject& bco, const CompilationContext& cc);

     private:
        String_t m_name;
        std::auto_ptr<Node> m_expr;
    };

} }

#endif
