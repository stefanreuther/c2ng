/**
  *  \file interpreter/expr/sequencenode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_SEQUENCENODE_HPP
#define C2NG_INTERPRETER_EXPR_SEQUENCENODE_HPP

#include "interpreter/expr/simplervaluenode.hpp"

namespace interpreter { namespace expr {

    /** Sequence node. Evaluates a, then b. */
    class SequenceNode : public SimpleRValueNode {
     public:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
    };

} }

#endif
