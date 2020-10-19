/**
  *  \file u/t_interpreter_selectionexpression.cpp
  *  \brief Test for interpreter::SelectionExpression
  */

#include "interpreter/selectionexpression.hpp"

#include "t_interpreter.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/error.hpp"

namespace {
    String_t compile(String_t expr)
    {
        interpreter::Tokenizer tok(expr);
        String_t result;
        interpreter::SelectionExpression::compile(tok, result);
        if (tok.getCurrentToken() != tok.tEnd) {
            throw interpreter::Error::garbageAtEnd(true);
        }
        return result;
    }
}

/** Test some valid expressions. */
void
TestInterpreterSelectionExpression::testValid()
{
    // Single operators
    TS_ASSERT_EQUALS(compile("a and b"), "AB&");
    TS_ASSERT_EQUALS(compile("a * b"), "AB&");
    TS_ASSERT_EQUALS(compile("a or b"), "AB|");
    TS_ASSERT_EQUALS(compile("a + b"), "AB|");
    TS_ASSERT_EQUALS(compile("a xor b"), "AB^");
    TS_ASSERT_EQUALS(compile("a - b"), "AB!&");

    // Unary
    TS_ASSERT_EQUALS(compile("-a"), "A!");
    TS_ASSERT_EQUALS(compile("not a"), "A!");

    // Combinations
    TS_ASSERT_EQUALS(compile("a and b or c"), "AB&C|");
    TS_ASSERT_EQUALS(compile("a or b and c"), "ABC&|");
    TS_ASSERT_EQUALS(compile("a and (b or c)"), "ABC|&");
    TS_ASSERT_EQUALS(compile("(a and b) or c"), "AB&C|");
    TS_ASSERT_EQUALS(compile("(a or b) and c"), "AB|C&");
    TS_ASSERT_EQUALS(compile("a or (b and c)"), "ABC&|");
    TS_ASSERT_EQUALS(compile("a and not b"), "AB!&");
    TS_ASSERT_EQUALS(compile("a and -b"), "AB!&");
    TS_ASSERT_EQUALS(compile("a - b"), "AB!&");

    // Literals
    TS_ASSERT_EQUALS(compile("current"), "c");
    TS_ASSERT_EQUALS(compile("1"), "1");
    TS_ASSERT_EQUALS(compile("0"), "0");
    TS_ASSERT_EQUALS(compile("ships"), "s");
    TS_ASSERT_EQUALS(compile("s"), "s");
    TS_ASSERT_EQUALS(compile("planets"), "p");
    TS_ASSERT_EQUALS(compile("p"), "p");

    // Masks
    TS_ASSERT_EQUALS(compile("s(a and b)"), "sAB&&");
    TS_ASSERT_EQUALS(compile("planets(e+f)"), "pEF|&");
    TS_ASSERT_EQUALS(compile("s and (a and b)"), "sAB&&");
    TS_ASSERT_EQUALS(compile("planets and (e+f)"), "pEF|&");
}

/** Test invalid expressions. */
void
TestInterpreterSelectionExpression::testInvalid()
{
    // False friends
    TS_ASSERT_THROWS(compile("true"), interpreter::Error);
    TS_ASSERT_THROWS(compile("false"), interpreter::Error);
    TS_ASSERT_THROWS(compile("ship"), interpreter::Error);
    TS_ASSERT_THROWS(compile("planet"), interpreter::Error);

    TS_ASSERT_THROWS(compile("a&b"), interpreter::Error);
    TS_ASSERT_THROWS(compile("a|b"), interpreter::Error);

    TS_ASSERT_THROWS(compile("@"), interpreter::Error);
    TS_ASSERT_THROWS(compile("i"), interpreter::Error);
    TS_ASSERT_THROWS(compile("2"), interpreter::Error);

    TS_ASSERT_THROWS(compile("a not b"), interpreter::Error);

    // Premature termination
    TS_ASSERT_THROWS(compile("(A"), interpreter::Error);
    TS_ASSERT_THROWS(compile("S(A"), interpreter::Error);
    TS_ASSERT_THROWS(compile("S()"), interpreter::Error);
    TS_ASSERT_THROWS(compile("a and"), interpreter::Error);

    // Too many
    TS_ASSERT_THROWS(compile("A)"), interpreter::Error);
    TS_ASSERT_THROWS(compile("S(A))"), interpreter::Error);
    TS_ASSERT_THROWS(compile("S)"), interpreter::Error);
}
