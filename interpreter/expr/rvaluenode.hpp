/**
  *  \file interpreter/expr/rvaluenode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_RVALUENODE_HPP
#define C2NG_INTERPRETER_EXPR_RVALUENODE_HPP

#include "interpreter/expr/node.hpp"

namespace interpreter { namespace expr {

    /** Generic r-value expression. This one refuses to be assigned-to. */
    class RValueNode : public Node {
     public:
        virtual void compileStore(BytecodeObject& bco, const CompilationContext& cc, Node& rhs);
        virtual void compileRead(BytecodeObject& bco, const CompilationContext& cc);
        virtual void compileWrite(BytecodeObject& bco, const CompilationContext& cc);
    };

} }

#endif
