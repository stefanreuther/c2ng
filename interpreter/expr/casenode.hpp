/**
  *  \file interpreter/expr/casenode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_CASENODE_HPP
#define C2NG_INTERPRETER_EXPR_CASENODE_HPP

#include "interpreter/expr/simplervaluenode.hpp"

namespace interpreter { namespace expr {

    /** Case-sensitive expression node. Generates a binary operation with the specified
        minor opcode when in case-sensitive mode, with minor+1 when case-insensitive.
        \todo Can we merge this somehow with IntCaseFunctionNode? */
    class CaseNode : public SimpleRValueNode {
     public:
        CaseNode(uint8_t minor);
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
        Node* convertToAssignment();

     private:
        uint8_t minor;
    };

} }

#endif
