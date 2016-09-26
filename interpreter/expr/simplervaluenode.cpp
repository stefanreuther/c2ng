/**
  *  \file interpreter/expr/simplervaluenode.cpp
  */

#include "interpreter/expr/simplervaluenode.hpp"

interpreter::expr::SimpleRValueNode::SimpleRValueNode()
    : a(0),
      b(0),
      c(0)
{ }

interpreter::expr::SimpleRValueNode::~SimpleRValueNode()
{
    delete a;
    delete b;
    delete c;
}

/** Set child nodes for unary expression. IntSimpleRValueExprNode will take ownership. */
void
interpreter::expr::SimpleRValueNode::setUnary(Node* a) throw()
{
    this->a = a;
}

/** Set child nodes for binary expression. IntSimpleRValueExprNode will take ownership. */
void
interpreter::expr::SimpleRValueNode::setBinary(Node* a, Node* b) throw()
{
    this->a = a;
    this->b = b;
}

/** Set child nodes for ternary expression. IntSimpleRValueExprNode will take ownership. */
void
interpreter::expr::SimpleRValueNode::setTernary(Node* a, Node* b, Node* c) throw()
{
    this->a = a;
    this->b = b;
    this->c = c;
}
