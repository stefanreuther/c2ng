/**
  *  \file interpreter/expr/literalnode.hpp
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
        LiteralNode();
        ~LiteralNode();
        void setNewValue(afl::data::Value* value) throw();
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);

     private:
        std::auto_ptr<afl::data::Value> m_value;
    };

} }

#endif
