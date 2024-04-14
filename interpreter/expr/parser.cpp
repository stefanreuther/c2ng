/**
  *  \file interpreter/expr/parser.cpp
  *  \brief Class interpreter::expr::Parser
  */

#include "interpreter/expr/parser.hpp"
#include "afl/string/format.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/assignmentnode.hpp"
#include "interpreter/expr/binarynode.hpp"
#include "interpreter/expr/builtinfunction.hpp"
#include "interpreter/expr/casenode.hpp"
#include "interpreter/expr/functioncallnode.hpp"
#include "interpreter/expr/identifiernode.hpp"
#include "interpreter/expr/indirectcallnode.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/expr/logicalnode.hpp"
#include "interpreter/expr/membernode.hpp"
#include "interpreter/expr/sequencenode.hpp"
#include "interpreter/expr/unarynode.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/values.hpp"

using afl::string::Format;

// Constructor.
interpreter::expr::Parser::Parser(Tokenizer& tok, afl::base::Deleter& del)
    : tok(tok),
      m_deleter(del)
{ }

// Parse expression.
const interpreter::expr::Node&
interpreter::expr::Parser::parse()
{
    // ex ccexpr.pas:GetExpr
    return parseSequence();
}

// Parse expression.
const interpreter::expr::Node&
interpreter::expr::Parser::parseNA()
{
    // ex ccexpr.pas:GetExprNA
    return parseOr();
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseSequence()
{
    // ex ccexpr.pas:NParse0
    // sequence ::= assignment
    //            | sequence ';' assignment
    const Node* p = &parseAssignment();
    while (tok.checkAdvance(Tokenizer::tSemicolon)) {
        // A line 'a := b;' will produce the error message "Expected operand" by default.
        // It is easy to generate a more helpful error message for this case (same as in PCC1),
        // this is not an additional grammar restriction.
        if (tok.getCurrentToken() == Tokenizer::tEnd) {
            throw Error("Lone \";\" at end of line is not allowed");
        }

        p = &m_deleter.addNew(new SequenceNode(*p, parseAssignment()));
    }
    return *p;
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseAssignment()
{
    // ex ccexpr.pas:NParse0b
    // @diff Different handling of file numbers, see parsePrimary().
    // assignment ::= or-expr
    //              | or-expr ':=' assignment
    const Node* p = &parseOr();
    if (tok.checkAdvance(Tokenizer::tAssign)) {
        p = &m_deleter.addNew(new AssignmentNode(*p, parseAssignment()));
    }
    return *p;
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseOr()
{
    // ex ccexpr.pas:NParse0a
    // or-expr ::= and-expr
    //           | or-expr 'Or' and-expr
    //           | or-expr 'Xor' and-expr
    const Node* p = &parseAnd();
    while (1) {
        if (tok.checkAdvance(Tokenizer::tOR)) {
            p = &m_deleter.addNew(new LogicalNode(Opcode::jIfTrue, biOr, *p, parseAnd()));
        } else if (tok.checkAdvance(Tokenizer::tXOR)) {
            p = &m_deleter.addNew(new LogicalNode(Opcode::jIfEmpty, biXor, *p, parseAnd()));
        } else {
            break;
        }
    }
    return *p;
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseAnd()
{
    // ex ccexpr.pas:NParse1
    // and-expr ::= not-expr
    //            | and-expr 'And' not-expr
    const Node* p = &parseNot();
    while (tok.checkAdvance(Tokenizer::tAND)) {
        p = &m_deleter.addNew(new LogicalNode(Opcode::jIfFalse, biAnd, *p, parseNot()));
    }
    return *p;
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseNot()
{
    // ex ccexpr.pas:NParse2
    // not-expr ::= comparison
    //            | 'Not' not-expr
    int n = 0;
    while (tok.checkAdvance(Tokenizer::tNOT)) {
        ++n;
    }
    const Node& result = parseComparison();
    if (n > 0) {
        if (n & 1) {
            // Negation
            return m_deleter.addNew(new UnaryNode(unNot, result));
        } else {
            // Cast to bool
            return m_deleter.addNew(new UnaryNode(unBool, result));
        }
    } else {
        return result;
    }
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseComparison()
{
    // ex ccexpr.pas:NParse3
    // comparison ::= concat-expr
    //              | comparison '=' concat-expr
    //              | comparison '<' concat-expr
    //              | comparison '>' concat-expr
    //              | comparison '<=' concat-expr
    //              | comparison '>=' concat-expr
    //              | comparison '<>' concat-expr
    const Node* p = &parseConcat();
    while (1) {
        uint8_t mode;
        if (tok.checkAdvance(Tokenizer::tEQ)) {
            mode = biCompareEQ;
        } else if (tok.checkAdvance(Tokenizer::tLT)) {
            mode = biCompareLT;
        } else if (tok.checkAdvance(Tokenizer::tGT)) {
            mode = biCompareGT;
        } else if (tok.checkAdvance(Tokenizer::tLE)) {
            mode = biCompareLE;
        } else if (tok.checkAdvance(Tokenizer::tGE)) {
            mode = biCompareGE;
        } else if (tok.checkAdvance(Tokenizer::tNE)) {
            mode = biCompareNE;
        } else {
            break;
        }
        p = &m_deleter.addNew(new CaseNode(mode, *p, parseConcat()));
    }
    return *p;
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseConcat()
{
    // ex ccexpr.pas:NParse4
    // concat-expr ::= add-expr
    //               | concat-expr "#" add-expr
    //               | concat-expr "&" add-expr
    const Node* p = &parseAdd();
    while (1) {
        BinaryOperation mode;
        if (tok.checkAdvance(Tokenizer::tHash)) {
            mode = biConcat;
        } else if (tok.checkAdvance(Tokenizer::tAmpersand)) {
            mode = biConcatEmpty;
        } else {
            break;
        }
        p = &m_deleter.addNew(new BinaryNode(mode, *p, parseAdd()));
    }
    return *p;
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseAdd()
{
    // ex ccexpr.pas:NParse5
    // add-expr ::= mult-expr
    //            | add-expr "+" mult-expr
    //            | add-expr "-" mult-expr
    const Node* p = &parseMult();
    while (1) {
        BinaryOperation mode;
        if (tok.checkAdvance(Tokenizer::tPlus)) {
            mode = biAdd;
        } else if (tok.checkAdvance(Tokenizer::tMinus)) {
            mode = biSub;
        } else {
            break;
        }
        p = &m_deleter.addNew(new BinaryNode(mode, *p, parseMult()));
    }
    return *p;
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseMult()
{
    // ex ccexpr.pas:NParse6
    // mult-expr ::= neg-expr
    //             | mult-expr "*" neg-expr
    //             | mult-expr "/" neg-expr
    //             | mult-expr "\" neg-expr
    //             | mult-expr "Mod" neg-expr
    const Node* p = &parseNeg();
    while (1) {
        BinaryOperation mode;
        if (tok.checkAdvance(Tokenizer::tMultiply)) {
            mode = biMult;
        } else if (tok.checkAdvance(Tokenizer::tSlash)) {
            mode = biDivide;
        } else if (tok.checkAdvance(Tokenizer::tBackslash)) {
            mode = biIntegerDivide;
        } else if (tok.checkAdvance(Tokenizer::tMOD)) {
            mode = biRemainder;
        } else {
            break;
        }
        p = &m_deleter.addNew(new BinaryNode(mode, *p, parseNeg()));
    }
    return *p;
}

const interpreter::expr::Node&
interpreter::expr::Parser::parseNeg()
{
    // ex ccexpr.pas:NParse7
    // neg-expr ::= pow-expr
    //            | "-" neg-expr
    //            | "+" neg-expr
    bool neg = false;
    bool had = false;
    while (1) {
        if (tok.checkAdvance(Tokenizer::tMinus)) {
            neg = !neg;
            had = true;
        } else if (tok.checkAdvance(Tokenizer::tPlus)) {
            had = true;
        } else {
            break;
        }
    }

    const Node& p = (tok.checkAdvance(Tokenizer::tNOT)
                     ? /* This rule makes PCC accept "-not x". This isn't part of the
                          original grammar, yet it's sensible in some way. Because it's
                          rare, we don't optimize here. */
                       m_deleter.addNew(new UnaryNode(unNot, parseNeg()))
                     : parsePow());

    if (had) {
        if (neg) {
            return m_deleter.addNew(new UnaryNode(unNeg, p));
        } else {
            return m_deleter.addNew(new UnaryNode(unPos, p));
        }
    } else {
        return p;
    }
}

const interpreter::expr::Node&
interpreter::expr::Parser::parsePow()
{
    // ex ccexpr.pas:NParse8
    // pow-expr ::= primary-expr
    //            | primary-expr "^" neg-expr
    const Node& p = parsePrimary();
    if (tok.checkAdvance(Tokenizer::tCaret)) {
        return m_deleter.addNew(new BinaryNode(biPow, p, parseNeg()));
    } else {
        return p;
    }
}

const interpreter::expr::Node&
interpreter::expr::Parser::parsePrimary()
{
    // ex ccexpr.pas:NParse9

    // primary-expr ::= "(" sequence ")"
    //                | literal
    //                | identifier {["(" arglist ")"] | ["." field]}*
    //
    // This grammar allows "a .b", but not "(a).b" for field references.
    // Although inconsistent with other programming languages, this is
    // actually a good thing, because it serves to disambiguate code like
    // "Ship(sid).Name := 'xxx'" (which would otherwise be ambiguous to
    // a procedure call with an assignment-expression as parameter:
    // "Ship (sid .Name := 'xxx')" */

    // @diff Different handling of file numbers between PCC 1.x and PCC2:
    // PCC 1.x parenthesizes "#a:=b" as "#(a:=b)", we parenthesize it as "(#a) := b".
    // Neither makes much sense so we accept that difference for now.
    if (tok.checkAdvance(Tokenizer::tLParen)) {
        // Parenthesized expression
        const Node& p = parseSequence();
        if (!tok.checkAdvance(Tokenizer::tRParen)) {
            throw Error::expectSymbol(")");
        }
        return p;
    } else if (tok.getCurrentToken() == Tokenizer::tInteger) {
        // Integer literal
        return makeLiteral(makeIntegerValue(tok.getCurrentInteger()));
    } else if (tok.getCurrentToken() == Tokenizer::tBoolean) {
        // Boolean literal
        return makeLiteral(makeBooleanValue(tok.getCurrentInteger()));
    } else if (tok.getCurrentToken() == Tokenizer::tFloat) {
        // Float literal
        return makeLiteral(makeFloatValue(tok.getCurrentFloat()));
    } else if (tok.getCurrentToken() == Tokenizer::tString) {
        // String literal
        return makeLiteral(makeStringValue(tok.getCurrentString()));
    } else if (tok.getCurrentToken() == Tokenizer::tIdentifier) {
        // Identifier
        String_t fname = tok.getCurrentString();

        // Special handling for builtin functions
        // ex ccexpr.pas:ParseCall
        const BuiltinFunctionDescriptor* bif;
        const Node* p;
        if (tok.readNextToken() == Tokenizer::tLParen && (bif = lookupBuiltinFunction(fname)) != 0) {
            // Builtin function
            tok.readNextToken();
            FunctionCallNode& fcn = m_deleter.addNew(bif->generator(*bif));
            parseArglist(fcn);
            // Basic checks
            if (fcn.getNumArgs() < bif->min_args) {
                throw Error::tooFewArguments(fname);
            }
            if (fcn.getNumArgs() > bif->max_args) {
                throw Error::tooManyArguments(fname);
            }
            p = &fcn;
        } else {
            // Regular function
            p = &m_deleter.addNew(new IdentifierNode(fname));
        }

        while (1) {
            if (tok.checkAdvance(Tokenizer::tLParen)) {
                // Array index / function call
                IndirectCallNode& fcn = m_deleter.addNew(new IndirectCallNode(*p));
                parseArglist(fcn);
                p = &fcn;
            } else if (tok.checkAdvance(Tokenizer::tDot) || tok.checkAdvance(Tokenizer::tArrow)) {
                // Member reference
                if (tok.getCurrentToken() != Tokenizer::tIdentifier) {
                    throw Error::expectIdentifier("field name");
                }
                p = &m_deleter.addNew(new MemberNode(tok.getCurrentString(), *p));
                tok.readNextToken();
            } else {
                break;
            }
        }
        return *p;
    } else if (tok.checkAdvance(Tokenizer::tHash)) {
        // File number
        return m_deleter.addNew(new UnaryNode(unFileNr, parsePrimary()));
    } else if (tok.getCurrentToken() == Tokenizer::tEnd) {
        throw Error("Expected operand");
    } else {
        throw Error("Invalid expression");
    }
}

/** Parse argument list. Arguments will be read and added to \c fcn.
    \param fcn Function call node to receive arguments */
void
interpreter::expr::Parser::parseArglist(FunctionCallNode& fcn)
{
    if (tok.checkAdvance(Tokenizer::tRParen)) {
        // "foo()"
    } else {
        // "foo(args...)"
        do {
            fcn.addArgument(parseSequence());
        } while (tok.checkAdvance(Tokenizer::tComma));
        if (!tok.checkAdvance(Tokenizer::tRParen)) {
            throw Error::expectSymbol(")");
        }
    }
}

const interpreter::expr::Node&
interpreter::expr::Parser::makeLiteral(afl::data::Value* pValue)
{
    LiteralNode& lit = m_deleter.addNew(new LiteralNode(std::auto_ptr<afl::data::Value>(pValue)));
    tok.readNextToken();
    return lit;
}
