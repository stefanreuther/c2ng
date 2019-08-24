/**
  *  \file interpreter/expr/parser.cpp
  *
  *  \todo FIXME: PCC 1.x has "#n" in the Assignment production. If we want to make
  *  it part of regular expressions, it probably makes more sense to have it
  *  in the Primary production. On the other hand, the most sensible solution
  *  probably is to implement it in the respective routine taking "#"-ed
  *  parameters.
  */

#include "interpreter/expr/parser.hpp"
#include "interpreter/expr/sequencenode.hpp"
#include "interpreter/expr/assignmentnode.hpp"
#include "interpreter/expr/logicalnode.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/expr/simplenode.hpp"
#include "interpreter/expr/casenode.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/expr/builtinfunction.hpp"
#include "interpreter/expr/functioncallnode.hpp"
#include "interpreter/expr/identifiernode.hpp"
#include "interpreter/expr/indirectcallnode.hpp"
#include "interpreter/expr/membernode.hpp"
#include "interpreter/error.hpp"
#include "afl/string/format.hpp"
#include "interpreter/values.hpp"

/** Constructor. */
interpreter::expr::Parser::Parser(Tokenizer& tok)
    : tok(tok),
      stack()
{ }

/** Parse expression. Parses a Sequence production.
    \return parsed expression tree. Caller is responsible for freeing it. */
interpreter::expr::Node*
interpreter::expr::Parser::parse()
{
    parseSequence();
    return stack.extractLast();
}

/** Parse expression. Parses an Or-Expr production (=no assignment, no sequence).
    \return parsed expression tree. Caller is responsible for freeing it. */
interpreter::expr::Node*
interpreter::expr::Parser::parseNA()
{
    parseOr();
    return stack.extractLast();
}

void
interpreter::expr::Parser::parseSequence()
{
    // sequence ::= assignment
    //            | sequence ';' assignment
    parseAssignment();
    while (tok.checkAdvance(tok.tSemicolon)) {
        parseAssignment();
        makeBinary(new SequenceNode());
    }
}

void
interpreter::expr::Parser::parseAssignment()
{
    // assignment ::= or-expr
    //              | or-expr ':=' assignment
    parseOr();
    if (tok.checkAdvance(tok.tAssign)) {
        parseAssignment();
        makeBinary(new AssignmentNode());
    }
}

void
interpreter::expr::Parser::parseOr()
{
    // or-expr ::= and-expr
    //           | or-expr 'Or' and-expr
    //           | or-expr 'Xor' and-expr
    parseAnd();
    while (1) {
        if (tok.checkAdvance(tok.tOR)) {
            parseAnd();
            makeBinary(new LogicalNode(Opcode::jIfTrue, interpreter::biOr));
        } else if (tok.checkAdvance(tok.tXOR)) {
            parseAnd();
            makeBinary(new LogicalNode(Opcode::jIfEmpty, interpreter::biXor));
        } else {
            break;
        }
    }
}

void
interpreter::expr::Parser::parseAnd()
{
    // and-expr ::= not-expr
    //            | and-expr 'And' not-expr
    parseNot();
    while (tok.checkAdvance(tok.tAND)) {
        parseNot();
        makeBinary(new LogicalNode(Opcode::jIfFalse, interpreter::biAnd));
    }
}

void
interpreter::expr::Parser::parseNot()
{
    // not-expr ::= comparison
    //            | 'Not' not-expr
    int n = 0;
    while (tok.checkAdvance(tok.tNOT)) {
        ++n;
    }
    parseComparison();
    if (n > 0) {
        if (n & 1) {
            // Negation
            makeUnary(new SimpleNode(Opcode::maUnary, interpreter::unNot));
        } else {
            // Cast to bool
            makeUnary(new SimpleNode(Opcode::maUnary, interpreter::unBool));
        }
    }
}

void
interpreter::expr::Parser::parseComparison()
{
    // comparison ::= concat-expr
    //              | comparison '=' concat-expr
    //              | comparison '<' concat-expr
    //              | comparison '>' concat-expr
    //              | comparison '<=' concat-expr
    //              | comparison '>=' concat-expr
    //              | comparison '<>' concat-expr
    parseConcat();
    while (1) {
        uint8_t mode;
        if (tok.checkAdvance(tok.tEQ)) {
            mode = interpreter::biCompareEQ;
        } else if (tok.checkAdvance(tok.tLT)) {
            mode = interpreter::biCompareLT;
        } else if (tok.checkAdvance(tok.tGT)) {
            mode = interpreter::biCompareGT;
        } else if (tok.checkAdvance(tok.tLE)) {
            mode = interpreter::biCompareLE;
        } else if (tok.checkAdvance(tok.tGE)) {
            mode = interpreter::biCompareGE;
        } else if (tok.checkAdvance(tok.tNE)) {
            mode = interpreter::biCompareNE;
        } else {
            break;
        }
        parseConcat();
        makeBinary(new CaseNode(mode));
    }
}

void
interpreter::expr::Parser::parseConcat()
{
    // concat-expr ::= add-expr
    //               | concat-expr "#" add-expr
    //               | concat-expr "&" add-expr
    parseAdd();
    while (1) {
        uint8_t mode;
        if (tok.checkAdvance(tok.tHash)) {
            mode = interpreter::biConcat;
        } else if (tok.checkAdvance(tok.tAmpersand)) {
            mode = interpreter::biConcatEmpty;
        } else {
            break;
        }
        parseAdd();
        makeBinary(new SimpleNode(Opcode::maBinary, mode));
    }
}

void
interpreter::expr::Parser::parseAdd()
{
    // add-expr ::= mult-expr
    //            | add-expr "+" mult-expr
    //            | add-expr "-" mult-expr
    parseMult();
    while (1) {
        uint8_t mode;
        if (tok.checkAdvance(tok.tPlus)) {
            mode = interpreter::biAdd;
        } else if (tok.checkAdvance(tok.tMinus)) {
            mode = interpreter::biSub;
        } else {
            break;
        }
        parseMult();
        makeBinary(new SimpleNode(Opcode::maBinary, mode));
    }
}

void
interpreter::expr::Parser::parseMult()
{
    // mult-expr ::= neg-expr
    //             | mult-expr "*" neg-expr
    //             | mult-expr "/" neg-expr
    //             | mult-expr "\" neg-expr
    //             | mult-expr "Mod" neg-expr
    parseNeg();
    while (1) {
        uint8_t mode;
        if (tok.checkAdvance(tok.tMultiply)) {
            mode = interpreter::biMult;
        } else if (tok.checkAdvance(tok.tSlash)) {
            mode = interpreter::biDivide;
        } else if (tok.checkAdvance(tok.tBackslash)) {
            mode = interpreter::biIntegerDivide;
        } else if (tok.checkAdvance(tok.tMOD)) {
            mode = interpreter::biRemainder;
        } else {
            break;
        }
        parseNeg();
        makeBinary(new SimpleNode(Opcode::maBinary, mode));
    }
}

void
interpreter::expr::Parser::parseNeg()
{
    // neg-expr ::= pow-expr
    //            | "-" neg-expr
    //            | "+" neg-expr
    bool neg = false;
    bool had = false;
    while (1) {
        if (tok.checkAdvance(tok.tMinus)) {
            neg = !neg;
            had = true;
        } else if (tok.checkAdvance(tok.tPlus)) {
            had = true;
        } else {
            break;
        }
    }

    if (tok.checkAdvance(tok.tNOT)) {
        /* This rule makes PCC accept "-not x". This isn't part of the
           original grammar, yet it's sensible in some way. Because it's
           rare, we don't optimize here. */
        parseNeg();
        makeUnary(new SimpleNode(Opcode::maUnary, interpreter::unNot));
    } else {
        parsePow();
    }

    if (had) {
        if (neg) {
            makeUnary(new SimpleNode(Opcode::maUnary, interpreter::unNeg));
        } else {
            makeUnary(new SimpleNode(Opcode::maUnary, interpreter::unPos));
        }
    }
}

void
interpreter::expr::Parser::parsePow()
{
    // pow-expr ::= primary-expr
    //            | primary-expr "^" neg-expr
    parsePrimary();
    if (tok.checkAdvance(tok.tCaret)) {
        parseNeg();
        makeBinary(new SimpleNode(Opcode::maBinary, interpreter::biPow));
    }
}

/* primary-expr ::= "(" sequence ")"
                  | literal
                  | identifier {["(" arglist ")"] | ["." field]}*

   This grammar allows "a .b", but not "(a).b" for field references.
   Although inconsistent with other programming languages, this is
   actually a good thing, because it serves to disambiguate code like
   "Ship(sid).Name := 'xxx'" (which would otherwise be ambiguous to
   a procedure call with an assignment-expression as parameter:
   "Ship (sid .Name := 'xxx')" */
void
interpreter::expr::Parser::parsePrimary()
{
    if (tok.checkAdvance(tok.tLParen)) {
        // Parenthesized expression
        parseSequence();
        if (!tok.checkAdvance(tok.tRParen)) {
            throw interpreter::Error::expectSymbol(")");
        }
    } else if (tok.getCurrentToken() == tok.tInteger || tok.getCurrentToken() == tok.tBoolean) {
        // Integer literal
        LiteralNode* lit = new LiteralNode;
        stack.pushBackNew(lit);
        lit->setNewValue(tok.getCurrentToken() == tok.tBoolean
                         ? makeBooleanValue(tok.getCurrentInteger())
                         : makeIntegerValue(tok.getCurrentInteger()));
        tok.readNextToken();
    } else if (tok.getCurrentToken() == tok.tFloat) {
        // Float literal
        LiteralNode* lit = new LiteralNode;
        stack.pushBackNew(lit);
        lit->setNewValue(makeFloatValue(tok.getCurrentFloat()));
        tok.readNextToken();
    } else if (tok.getCurrentToken() == tok.tString) {
        // String literal
        LiteralNode* lit = new LiteralNode;
        stack.pushBackNew(lit);
        lit->setNewValue(makeStringValue(tok.getCurrentString()));
        tok.readNextToken();
    } else if (tok.getCurrentToken() == tok.tIdentifier) {
        // Identifier
        String_t fname = tok.getCurrentString();

        // Special handling for builtin functions
        const BuiltinFunctionDescriptor* bif;
        if (tok.readNextToken() == tok.tLParen && (bif = lookupBuiltinFunction(fname)) != 0) {
            // Builtin function
            tok.readNextToken();
            FunctionCallNode* fcn = bif->generator(*bif);
            stack.pushBackNew(fcn);
            parseArglist(fcn);
            // Basic checks
            if (fcn->getNumArgs() < bif->min_args) {
                throw interpreter::Error(afl::string::Format("Too few arguments for \"%s\"", fname)); // FIXME
            }
            if (fcn->getNumArgs() > bif->max_args) {
                throw interpreter::Error(afl::string::Format("Too many arguments for \"%s\"", fname)); // FIXME
            }
        } else {
            // Regular function
            stack.pushBackNew(new IdentifierNode(fname));
        }
        while (1) {
            if (tok.checkAdvance(tok.tLParen)) {
                // Array index / function call
                IndirectCallNode* fcn = new IndirectCallNode();
                fcn->setNewFunction(stack.extractLast());
                stack.pushBackNew(fcn);
                parseArglist(fcn);
            } else if (tok.checkAdvance(tok.tDot) || tok.checkAdvance(tok.tArrow)) {
                // Member reference
                if (tok.getCurrentToken() != tok.tIdentifier) {
                    throw interpreter::Error::expectIdentifier("field name");
                }
                MemberNode* mem = new MemberNode(tok.getCurrentString());
                mem->setNewExpression(stack.extractLast());
                stack.pushBackNew(mem);
                tok.readNextToken();
            } else {
                break;
            }
        }
    } else if (tok.checkAdvance(tok.tHash)) {
        // File number
        parsePrimary();
        makeUnary(new SimpleNode(Opcode::maUnary, interpreter::unFileNr));
    } else if (tok.getCurrentToken() == tok.tEnd) {
        throw interpreter::Error("Expected operand");
    } else {
        throw interpreter::Error("Invalid expression");
    }
}

/** Parse argument list. Arguments will be read and added to \c fcn.
    \c fcn must already be on the stack, for exception safety.
    \param fcn Function call node to receive arguments */
void
interpreter::expr::Parser::parseArglist(FunctionCallNode* fcn)
{
    if (tok.checkAdvance(tok.tRParen)) {
        // "foo()"
    } else {
        // "foo(args...)"
        do {
            parseSequence();
            fcn->addNewArgument(stack.extractLast());
        } while (tok.checkAdvance(tok.tComma));
        if (!tok.checkAdvance(tok.tRParen)) {
            throw interpreter::Error::expectSymbol(")");
        }
    }
}

/** Make unary operation. Expects one tree node on the stack.
    \param n Tree node, will be pushed onto the stack */
void
interpreter::expr::Parser::makeUnary(SimpleRValueNode* n)
{
    Node* a = stack.extractLast();
    n->setUnary(a);
    stack.pushBackNew(n);
}

/** Make binary operation. Expects two tree nodes on the stack.
    \param n Tree node, will be pushed onto the stack */
void
interpreter::expr::Parser::makeBinary(SimpleRValueNode* n)
{
    Node* b = stack.extractLast();
    Node* a = stack.extractLast();
    n->setBinary(a, b);
    stack.pushBackNew(n);
}
