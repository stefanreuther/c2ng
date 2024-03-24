/**
  *  \file test/interpreter/test/expressionverifiertest.cp
  *  \brief Test for interpreter::test::ExpressionVerifier
  *
  *  This test (obviously) tests part of the code it itself is intended to test.
  */

#include "interpreter/test/expressionverifier.hpp"

#include "afl/except/assertionfailedexception.hpp"
#include "afl/test/testrunner.hpp"

using interpreter::test::ExpressionVerifier;
using afl::except::AssertionFailedException;

AFL_TEST("interpreter.test.ExpressionVerifier:init", a)
{
    ExpressionVerifier testee(a);
    a.checkEqual("01", testee.get(0), 0);

    testee.set(0, 10);
    a.checkEqual("11", testee.get(0), 10);

    testee.clear();
    a.checkEqual("21", testee.get(0), 0);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyInteger", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyInteger"), testee.verifyInteger("1", 1));
    AFL_CHECK_THROWS  (a("02. verifyInteger"), testee.verifyInteger("'1'",  1), AssertionFailedException);
    AFL_CHECK_THROWS  (a("03. verifyInteger"), testee.verifyInteger("(",    1), AssertionFailedException);
    AFL_CHECK_THROWS  (a("04. verifyInteger"), testee.verifyInteger("True", 1), AssertionFailedException);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyBoolean", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyBoolean"), testee.verifyBoolean("True", true));
    AFL_CHECK_THROWS  (a("02. verifyBoolean"), testee.verifyBoolean("'1'",  1), AssertionFailedException);
    AFL_CHECK_THROWS  (a("03. verifyBoolean"), testee.verifyBoolean("(",    1), AssertionFailedException);
    AFL_CHECK_THROWS  (a("04. verifyBoolean"), testee.verifyBoolean("1",    1), AssertionFailedException);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyFile", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyFile"), testee.verifyFile("#7", 7));
    AFL_CHECK_THROWS  (a("02. verifyFile"), testee.verifyFile("7",    7), AssertionFailedException);
    AFL_CHECK_THROWS  (a("03. verifyFile"), testee.verifyFile("(",    7), AssertionFailedException);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyNull", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyNull"), testee.verifyNull("Z(0)"));
    AFL_CHECK_THROWS  (a("02. verifyNull"), testee.verifyNull("7"), AssertionFailedException);
    AFL_CHECK_THROWS  (a("03. verifyNull"), testee.verifyNull("("), AssertionFailedException);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyString", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyString"), testee.verifyString("'a'", "a"));
    AFL_CHECK_THROWS  (a("02. verifyString"), testee.verifyString("1", "a"), AssertionFailedException);
    AFL_CHECK_THROWS  (a("03. verifyString"), testee.verifyString("(", "a"), AssertionFailedException);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyFloat", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyFloat"), testee.verifyFloat("1.5", 1.5));
    AFL_CHECK_THROWS  (a("02. verifyFloat"), testee.verifyFloat("1", 1.0), AssertionFailedException);
    AFL_CHECK_THROWS  (a("03. verifyFloat"), testee.verifyFloat("(", 1.0), AssertionFailedException);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyExecutionError", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyExecutionError"), testee.verifyExecutionError("QQ"));
    AFL_CHECK_THROWS  (a("02. verifyExecutionError"), testee.verifyExecutionError("("), AssertionFailedException);
    AFL_CHECK_THROWS  (a("03. verifyExecutionError"), testee.verifyExecutionError("1"), AssertionFailedException);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyCompileError", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyCompileError"), testee.verifyCompileError("ByName(1)"));
    AFL_CHECK_THROWS  (a("02. verifyCompileError"), testee.verifyCompileError("("), AssertionFailedException);
    AFL_CHECK_THROWS  (a("03. verifyCompileError"), testee.verifyCompileError("1"), AssertionFailedException);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyParseError", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyParseError"), testee.verifyParseError("("));
    AFL_CHECK_SUCCEEDS(a("02. verifyParseError"), testee.verifyParseError("a'"));
    AFL_CHECK_SUCCEEDS(a("03. verifyParseError"), testee.verifyParseError("'"));
    AFL_CHECK_THROWS  (a("04. verifyParseError"), testee.verifyParseError("1"), AssertionFailedException);
}

AFL_TEST("interpreter.test.ExpressionVerifier:verifyStatement", a)
{
    ExpressionVerifier testee(a);
    AFL_CHECK_SUCCEEDS(a("01. verifyStatement"), testee.verifyStatement("a:=1"));
    AFL_CHECK_THROWS  (a("02. verifyStatement"), testee.verifyStatement("("), AssertionFailedException);
    AFL_CHECK_THROWS  (a("03. verifyStatement"), testee.verifyStatement("a:=b/c"), AssertionFailedException);   // divide by zero
}
