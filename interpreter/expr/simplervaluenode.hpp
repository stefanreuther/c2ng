/**
  *  \file interpreter/expr/simplervaluenode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_SIMPLERVALUENODE_HPP
#define C2NG_INTERPRETER_EXPR_SIMPLERVALUENODE_HPP

#include "interpreter/expr/rvaluenode.hpp"

namespace interpreter { namespace expr {

    /** Basic rvalue node.
        This is an internal node with up to three child nodes. Code generation is implemented in derived classes. */
    class SimpleRValueNode : public RValueNode {
     public:
        SimpleRValueNode();
        ~SimpleRValueNode();

        void setUnary(Node* a) throw();
        void setBinary(Node* a, Node* b) throw();
        void setTernary(Node* a, Node* b, Node* c) throw();

     protected:
        // FIXME: rewrite this!
        Node* a;
        Node* b;
        Node* c;
    };

} }

#endif
