/**
  *  \file interpreter/expr/identifiernode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_IDENTIFIERNODE_HPP
#define C2NG_INTERPRETER_EXPR_IDENTIFIERNODE_HPP

#include "interpreter/expr/node.hpp"

namespace interpreter { namespace expr {

    /** Identifier access. Implements a freestanding identifier ("x"). */
    class IdentifierNode : public Node {
     public:
        IdentifierNode(String_t name);
        ~IdentifierNode();
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc);
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileStore(BytecodeObject& bco, const CompilationContext& cc, Node& rhs);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
        void compileRead(BytecodeObject& bco, const CompilationContext& cc);
        void compileWrite(BytecodeObject& bco, const CompilationContext& cc);
        const String_t& getIdentifier() const
            { return name; }

     private:
        String_t name;
    };

} }

#endif
