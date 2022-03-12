/**
  *  \file interpreter/expr/functioncallnode.hpp
  *  \brief Class interpreter::expr::FunctionCallNode
  */
#ifndef C2NG_INTERPRETER_EXPR_FUNCTIONCALLNODE_HPP
#define C2NG_INTERPRETER_EXPR_FUNCTIONCALLNODE_HPP

#include <vector>
#include "interpreter/expr/rvaluenode.hpp"

namespace interpreter { namespace expr {

    /** Function call.
        Base class for all sorts of invocations ("something(args)"). */
    class FunctionCallNode : public RValueNode {
     public:
        /** Append argument.
            @param arg Argument, newly-allocated */
        void addArgument(const Node& arg);

        /** Get number of arguments.
            @return number */
        size_t getNumArgs() const
            { return args.size(); }

     protected:
        std::vector<const Node*> args;
    };

} }

#endif
