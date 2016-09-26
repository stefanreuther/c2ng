/**
  *  \file interpreter/expr/conditionalnode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_CONDITIONALNODE_HPP
#define C2NG_INTERPRETER_EXPR_CONDITIONALNODE_HPP

#include "interpreter/expr/simplervaluenode.hpp"

namespace interpreter { namespace expr {

    /** If/then/else node. */
    class ConditionalNode : public SimpleRValueNode {
     public:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
    };

} }

#endif
