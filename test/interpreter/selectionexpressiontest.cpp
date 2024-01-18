/**
  *  \file test/interpreter/selectionexpressiontest.cpp
  *  \brief Test for interpreter::SelectionExpression
  */

#include "interpreter/selectionexpression.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tokenizer.hpp"

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
AFL_TEST("interpreter.SelectionExpression:valid", a)
{
    // Single operators
    a.checkEqual("01", compile("a and b"), "AB&");
    a.checkEqual("02", compile("a * b"), "AB&");
    a.checkEqual("03", compile("a or b"), "AB|");
    a.checkEqual("04", compile("a + b"), "AB|");
    a.checkEqual("05", compile("a xor b"), "AB^");
    a.checkEqual("06", compile("a - b"), "AB!&");

    // Unary
    a.checkEqual("11", compile("-a"), "A!");
    a.checkEqual("12", compile("not a"), "A!");

    // Combinations
    a.checkEqual("21", compile("a and b or c"), "AB&C|");
    a.checkEqual("22", compile("a or b and c"), "ABC&|");
    a.checkEqual("23", compile("a and (b or c)"), "ABC|&");
    a.checkEqual("24", compile("(a and b) or c"), "AB&C|");
    a.checkEqual("25", compile("(a or b) and c"), "AB|C&");
    a.checkEqual("26", compile("a or (b and c)"), "ABC&|");
    a.checkEqual("27", compile("a and not b"), "AB!&");
    a.checkEqual("28", compile("a and -b"), "AB!&");
    a.checkEqual("29", compile("a - b"), "AB!&");

    // Literals
    a.checkEqual("31", compile("current"), "c");
    a.checkEqual("32", compile("1"), "1");
    a.checkEqual("33", compile("0"), "0");
    a.checkEqual("34", compile("ships"), "s");
    a.checkEqual("35", compile("s"), "s");
    a.checkEqual("36", compile("planets"), "p");
    a.checkEqual("37", compile("p"), "p");

    // Masks
    a.checkEqual("41", compile("s(a and b)"), "sAB&&");
    a.checkEqual("42", compile("planets(e+f)"), "pEF|&");
    a.checkEqual("43", compile("s and (a and b)"), "sAB&&");
    a.checkEqual("44", compile("planets and (e+f)"), "pEF|&");
}

/** Test invalid expressions. */
AFL_TEST("interpreter.SelectionExpression:error", a)
{
    // False friends
    AFL_CHECK_THROWS(a("r01"), compile("true"), interpreter::Error);
    AFL_CHECK_THROWS(a("r02"), compile("false"), interpreter::Error);
    AFL_CHECK_THROWS(a("r03"), compile("ship"), interpreter::Error);
    AFL_CHECK_THROWS(a("r04"), compile("planet"), interpreter::Error);

    AFL_CHECK_THROWS(a("r11"), compile("a&b"), interpreter::Error);
    AFL_CHECK_THROWS(a("r12"), compile("a|b"), interpreter::Error);

    AFL_CHECK_THROWS(a("r21"), compile("@"), interpreter::Error);
    AFL_CHECK_THROWS(a("r22"), compile("i"), interpreter::Error);
    AFL_CHECK_THROWS(a("r23"), compile("2"), interpreter::Error);

    AFL_CHECK_THROWS(a("r31"), compile("a not b"), interpreter::Error);

    // Premature termination
    AFL_CHECK_THROWS(a("r41"), compile("(A"), interpreter::Error);
    AFL_CHECK_THROWS(a("r42"), compile("S(A"), interpreter::Error);
    AFL_CHECK_THROWS(a("r43"), compile("S()"), interpreter::Error);
    AFL_CHECK_THROWS(a("r44"), compile("a and"), interpreter::Error);

    // Too many
    AFL_CHECK_THROWS(a("r51"), compile("A)"), interpreter::Error);
    AFL_CHECK_THROWS(a("r52"), compile("S(A))"), interpreter::Error);
    AFL_CHECK_THROWS(a("r53"), compile("S)"), interpreter::Error);
}
