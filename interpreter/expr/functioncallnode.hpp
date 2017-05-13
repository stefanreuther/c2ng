/**
  *  \file interpreter/expr/functioncallnode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_FUNCTIONCALLNODE_HPP
#define C2NG_INTERPRETER_EXPR_FUNCTIONCALLNODE_HPP

#include "afl/container/ptrvector.hpp"
#include "interpreter/expr/rvaluenode.hpp"

namespace interpreter { namespace expr {

    /** Function call. Base class for all sorts of invocations ("something(args)"). */
    class FunctionCallNode : public RValueNode {
     public:
        void addNewArgument(Node* arg);
        size_t getNumArgs() const
            { return args.size(); }
     protected:
        // FIXME: un-export this!
        afl::container::PtrVector<Node> args;
    };

} }

#endif
