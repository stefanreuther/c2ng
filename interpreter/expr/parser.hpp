/**
  *  \file interpreter/expr/parser.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_PARSER_HPP
#define C2NG_INTERPRETER_EXPR_PARSER_HPP

#include "interpreter/tokenizer.hpp"
#include "afl/container/ptrvector.hpp"

namespace interpreter { namespace expr {

    class Node;
    class FunctionCallNode;
    class SimpleRValueNode;

    /** Expression Parser. This parses an expression, tokenized by an IntTokenizer,
        into a tree of IntExprNode's.

        To support exception safety, this collects expression trees on a stack which
        is cleaned up in the destructor. During parsing, several partial trees exist
        on the stack. When parsing finishes, only one tree (the result) remains, which
        refers to all former partial trees. */
    class Parser {
     public:
        Parser(Tokenizer& tok);

        Node* parse();
        Node* parseNA();

     private:
        void parseSequence();
        void parseAssignment();
        void parseOr();
        void parseAnd();
        void parseNot();
        void parseComparison();
        void parseConcat();
        void parseAdd();
        void parseMult();
        void parseNeg();
        void parsePow();
        void parsePrimary();
        void parseArglist(FunctionCallNode* fcn);

        void makeUnary(SimpleRValueNode* n);
        void makeBinary(SimpleRValueNode* n);
        void makeTernary(SimpleRValueNode* n);

        Tokenizer& tok;
        afl::container::PtrVector<Node> stack;
    };

} }

#endif
