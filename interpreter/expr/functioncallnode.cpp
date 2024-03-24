/**
  *  \file interpreter/expr/functioncallnode.cpp
  *  \brief Class interpreter::expr::FunctionCallNode
  */

#include "interpreter/expr/functioncallnode.hpp"

void
interpreter::expr::FunctionCallNode::addArgument(const Node& arg)
{
    // ex IntFunctionCallNode::addArgument
    m_args.push_back(&arg);
}
