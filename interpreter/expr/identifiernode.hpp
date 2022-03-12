/**
  *  \file interpreter/expr/identifiernode.hpp
  *  \brief Class interpreter::expr::IdentifierNode
  */
#ifndef C2NG_INTERPRETER_EXPR_IDENTIFIERNODE_HPP
#define C2NG_INTERPRETER_EXPR_IDENTIFIERNODE_HPP

#include "interpreter/expr/node.hpp"

namespace interpreter { namespace expr {

    /** Identifier access. Implements a freestanding identifier ("x"). */
    class IdentifierNode : public Node {
     public:
        /** Constructor.
            @param name Identifier */
        IdentifierNode(String_t name);

        /** Destructor. */
        ~IdentifierNode();

        // Node:
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileStore(BytecodeObject& bco, const CompilationContext& cc, const Node& rhs) const;
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;
        void compileRead(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileWrite(BytecodeObject& bco, const CompilationContext& cc) const;

        /** Get identifier.
            @return identifier */
        const String_t& getIdentifier() const
            { return m_name; }

     private:
        String_t m_name;
    };

} }

#endif
