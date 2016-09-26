/**
  *  \file interpreter/expr/assignmentnode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_ASSIGNMENTNODE_HPP
#define C2NG_INTERPRETER_EXPR_ASSIGNMENTNODE_HPP

#include "interpreter/expr/simplervaluenode.hpp"

namespace interpreter { namespace expr {

    /** Assignment node. Assigns node b to node a. */
    class AssignmentNode : public SimpleRValueNode {
     public:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
    };

} }

#endif
