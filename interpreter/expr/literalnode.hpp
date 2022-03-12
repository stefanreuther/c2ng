/**
  *  \file interpreter/expr/literalnode.hpp
  *  \brief Class interpreter::expr::LiteralNode
  */
#ifndef C2NG_INTERPRETER_EXPR_LITERALNODE_HPP
#define C2NG_INTERPRETER_EXPR_LITERALNODE_HPP

#include <memory>
#include "interpreter/expr/rvaluenode.hpp"
#include "afl/data/value.hpp"

namespace interpreter { namespace expr {

    /** Literal expression node. Generates code to return a literal value. */
    class LiteralNode : public RValueNode {
     public:
        /** Constructor. */
        LiteralNode();
        ~LiteralNode();

        /** Set value.
            For now, we use two-step construction to ensure exception safety.
            @param value New value. LiteralNode will take ownership. */
        void setNewValue(afl::data::Value* value) throw();

        // Node:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc) const;
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;

        /** Get value.
            @return value */
        afl::data::Value* getValue() const
            { return m_value.get(); }

     private:
        std::auto_ptr<afl::data::Value> m_value;
    };

} }

#endif
