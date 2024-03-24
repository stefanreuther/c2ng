/**
  *  \file interpreter/expr/indirectcallnode.hpp
  *  \brief Class interpreter::expr::IndirectCallNode
  */
#ifndef C2NG_INTERPRETER_EXPR_INDIRECTCALLNODE_HPP
#define C2NG_INTERPRETER_EXPR_INDIRECTCALLNODE_HPP

#include "interpreter/expr/functioncallnode.hpp"

namespace interpreter { namespace expr {

    /** Indirect (=user-defined or member) function call.
        Implements general "someexpr(args)". */
    class IndirectCallNode : public FunctionCallNode {
     public:
        /** Constructor.
            @param func Function to call */
        IndirectCallNode(const Node& func);

        // Node:
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileStore(BytecodeObject& bco, const CompilationContext& cc, const Node& rhs) const;
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;
        void compileRead(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileWrite(BytecodeObject& bco, const CompilationContext& cc) const;

     private:
        const Node& m_function;
    };

} }

#endif
