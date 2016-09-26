/**
  *  \file interpreter/expr/indirectcallnode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_INDIRECTCALLNODE_HPP
#define C2NG_INTERPRETER_EXPR_INDIRECTCALLNODE_HPP

#include "interpreter/expr/functioncallnode.hpp"

namespace interpreter { namespace expr {

    /** Indirect (=user-defined or member) function call.
        Implements general "someexpr(args)". */
    class IndirectCallNode : public FunctionCallNode {
     public:
        IndirectCallNode();
        ~IndirectCallNode();
        void setNewFunction(Node* func);

        // FIXME: can we implement all these members or do we have some already up-tree?
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc);
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileStore(BytecodeObject& bco, const CompilationContext& cc, Node& rhs);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
        void compileRead(BytecodeObject& bco, const CompilationContext& cc);
        void compileWrite(BytecodeObject& bco, const CompilationContext& cc);
     private:
        Node* func;
    };

} }

#endif
