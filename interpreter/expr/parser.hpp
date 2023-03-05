/**
  *  \file interpreter/expr/parser.hpp
  *  \brief Class interpreter::expr::Parser
  */
#ifndef C2NG_INTERPRETER_EXPR_PARSER_HPP
#define C2NG_INTERPRETER_EXPR_PARSER_HPP

#include "afl/base/deleter.hpp"
#include "afl/data/value.hpp"
#include "interpreter/tokenizer.hpp"

namespace interpreter { namespace expr {

    class Node;
    class FunctionCallNode;

    /** Expression Parser.
        This parses an expression, tokenized by a Tokenizer, into a tree of Node's.

        Nodes are collected in a Deleter and live as long as that.
        Unlike in PCC2, nodes do no longer control lifetime of other nodes. */
    class Parser {
     public:
        /** Constructor.
            @param tok  Tokenizer
            @param del  Deleter to contain created nodes */
        Parser(Tokenizer& tok, afl::base::Deleter& del);

        /** Parse expression. Parses a "Sequence" production.
            @return parsed expression tree */
        const Node& parse();

        /** Parse expression. Parses an "Or-Expr" production (=no assignment, no sequence).
            @return parsed expression tree */
        const Node& parseNA();

     private:
        const Node& parseSequence();
        const Node& parseAssignment();
        const Node& parseOr();
        const Node& parseAnd();
        const Node& parseNot();
        const Node& parseComparison();
        const Node& parseConcat();
        const Node& parseAdd();
        const Node& parseMult();
        const Node& parseNeg();
        const Node& parsePow();
        const Node& parsePrimary();
        void parseArglist(FunctionCallNode& fcn);

        Tokenizer& tok;
        afl::base::Deleter& m_deleter;

        const Node& makeLiteral(afl::data::Value* pValue);
    };

} }

#endif
