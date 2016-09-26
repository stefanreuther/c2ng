/**
  *  \file interpreter/expr/functioncallnode.cpp
  */

#include "interpreter/expr/functioncallnode.hpp"

/** Append argument.
    \param arg Argument, newly-allocated */
void
interpreter::expr::FunctionCallNode::addNewArgument(Node* arg)
{
    // ex IntFunctionCallNode::addArgument
    args.pushBackNew(arg);
}
