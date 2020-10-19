/**
  *  \file interpreter/selectionexpression.cpp
  *  \brief Class interpreter::SelectionExpression
  */

#include "interpreter/selectionexpression.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tokenizer.hpp"

// Compile selection expression.
void
interpreter::SelectionExpression::compile(Tokenizer& tok, String_t& expr)
{
    // ex int/selexpr.h:compileSelectionExpression
    compileSummand(tok, expr);
    while (1) {
        if (tok.checkAdvance(tok.tOR) || tok.checkAdvance(tok.tPlus)) {
            compileSummand(tok, expr);
            expr += opOr;
        } else if (tok.checkAdvance(tok.tXOR)) {
            compileSummand(tok, expr);
            expr += opXor;
        } else if (tok.checkAdvance(tok.tMinus)) {
            compileSummand(tok, expr);
            expr += opNot;
            expr += opAnd;
        } else {
            break;
        }
    }
}

/** Compile optional type-mask in set expression.
    A parenthesized expression after "S" or "P" means to select all ships or planets from that expression. */
void
interpreter::SelectionExpression::compileOptionalTypeMask(Tokenizer& tok, String_t& expr)
{
    // ex int/selexpr.cc:compileOptionalTypeMask
    if (tok.checkAdvance(tok.tLParen)) {
        compile(tok, expr);
        expr += opAnd;
        if (!tok.checkAdvance(tok.tRParen)) {
            throw Error::expectSymbol(")");
        }
    }
}

/** Compile "factor" for set expression.

    factor ::= "not" factor
             | "-" factor                 // this production not in PCC 1.x
             | "S" type-mask_opt
             | "SHIPS" type-mask_opt
             | "P" type-mask_opt
             | "PLANETS" type-mask_opt
             | "CURRENT"
             | "(" expression ")"
             | "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H"
             | "0" | "1" */
void
interpreter::SelectionExpression::compileFactor(Tokenizer& tok, String_t& expr)
{
    // ex search.pas::CompileFactor
    // ex int/selexpr.cc:compileFactor
    if (tok.checkAdvance(tok.tNOT) || tok.checkAdvance(tok.tMinus)) {
        compileFactor(tok, expr);
        expr += opNot;
    } else if (tok.checkAdvance("S") || tok.checkAdvance("SHIPS")) {
        expr += opShip;
        compileOptionalTypeMask(tok, expr);
    } else if (tok.checkAdvance("P") || tok.checkAdvance("PLANETS")) {
        expr += opPlanet;
        compileOptionalTypeMask(tok, expr);
    } else if (tok.checkAdvance("CURRENT")) {
        expr += opCurrent;
    } else if (tok.checkAdvance(tok.tLParen)) {
        compile(tok, expr);
        if (!tok.checkAdvance(tok.tRParen)) {
            throw Error::expectSymbol(")");
        }
    } else {
        if (tok.getCurrentToken() == tok.tIdentifier) {
            String_t s = tok.getCurrentString();
            if (s.size() == 1 && s[0] >= 'A' && s[0] < 'A' + NUM_SELECTION_LAYERS) {
                expr += char(s[0] - 'A' + opFirstLayer);
                tok.readNextToken();
            } else {
                throw Error::unknownIdentifier(s);
            }
        } else if (tok.getCurrentToken() == tok.tInteger) {
            if (tok.getCurrentInteger() == 0) {
                expr += opZero;
                tok.readNextToken();
            } else if (tok.getCurrentInteger() == 1) {
                expr += opOne;
                tok.readNextToken();
            } else {
                throw Error("Invalid operand in set expression");
            }
        } else if (tok.getCurrentToken() == tok.tEnd) {
            throw Error("Expecting operand in set expression");
        } else {
            throw Error("Invalid operand in set expression");
        }
    } 
}

/** Compile "summand" for set expression.

    summand := factor
             | summand "And" factor
             | summand "*" factor */
void
interpreter::SelectionExpression::compileSummand(Tokenizer& tok, String_t& expr)
{
    // ex int/selexpr.cc:compileSummand
    compileFactor(tok, expr);
    while (1) {
        if (tok.checkAdvance(tok.tAND) || tok.checkAdvance(tok.tMultiply)) {
            compileFactor(tok, expr);
            expr += opAnd;
        } else {
            break;
        }
    }
}
