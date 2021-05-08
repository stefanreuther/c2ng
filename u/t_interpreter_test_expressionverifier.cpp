/**
  *  \file u/t_interpreter_test_expressionverifier.cpp
  *  \brief Test for interpreter::test::ExpressionVerifier
  *
  *  This test (obviously) tests part of the code it itself is intended to test.
  */

#include "interpreter/test/expressionverifier.hpp"

#include "t_interpreter_test.hpp"
#include "afl/except/assertionfailedexception.hpp"

using interpreter::test::ExpressionVerifier;
using afl::except::AssertionFailedException;

void
TestInterpreterTestExpressionVerifier::testInit()
{
    ExpressionVerifier testee("testInit");
    TS_ASSERT_EQUALS(testee.get(0), 0);

    testee.set(0, 10);
    TS_ASSERT_EQUALS(testee.get(0), 10);

    testee.clear();
    TS_ASSERT_EQUALS(testee.get(0), 0);
}

void
TestInterpreterTestExpressionVerifier::testVerifyInteger()
{
    ExpressionVerifier testee("testVerifyInteger");
    TS_ASSERT_THROWS_NOTHING(testee.verifyInteger("1", 1));
    TS_ASSERT_THROWS(testee.verifyInteger("'1'",  1), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyInteger("(",    1), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyInteger("True", 1), AssertionFailedException);
}

void
TestInterpreterTestExpressionVerifier::testVerifyBoolean()
{
    ExpressionVerifier testee("testVerifyBoolean");
    TS_ASSERT_THROWS_NOTHING(testee.verifyBoolean("True", true));
    TS_ASSERT_THROWS(testee.verifyBoolean("'1'",  1), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyBoolean("(",    1), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyBoolean("1",    1), AssertionFailedException);
}

void
TestInterpreterTestExpressionVerifier::testVerifyFile()
{
    ExpressionVerifier testee("testVerifyFile");
    TS_ASSERT_THROWS_NOTHING(testee.verifyFile("#7", 7));
    TS_ASSERT_THROWS(testee.verifyFile("7",    7), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyFile("(",    7), AssertionFailedException);
}

void
TestInterpreterTestExpressionVerifier::testVerifyNull()
{
    ExpressionVerifier testee("testVerifyNull");
    TS_ASSERT_THROWS_NOTHING(testee.verifyNull("Z(0)"));
    TS_ASSERT_THROWS(testee.verifyNull("7"), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyNull("("), AssertionFailedException);
}

void
TestInterpreterTestExpressionVerifier::testVerifyString()
{
    ExpressionVerifier testee("testVerifyString");
    TS_ASSERT_THROWS_NOTHING(testee.verifyString("'a'", "a"));
    TS_ASSERT_THROWS(testee.verifyString("1", "a"), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyString("(", "a"), AssertionFailedException);
}

void
TestInterpreterTestExpressionVerifier::testVerifyFloat()
{
    ExpressionVerifier testee("testVerifyFloat");
    TS_ASSERT_THROWS_NOTHING(testee.verifyFloat("1.5", 1.5));
    TS_ASSERT_THROWS(testee.verifyFloat("1", 1.0), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyFloat("(", 1.0), AssertionFailedException);
}

void
TestInterpreterTestExpressionVerifier::testVerifyExecutionError()
{
    ExpressionVerifier testee("testVerifyExecutionError");
    TS_ASSERT_THROWS_NOTHING(testee.verifyExecutionError("QQ"));
    TS_ASSERT_THROWS(testee.verifyExecutionError("("), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyExecutionError("1"), AssertionFailedException);
}

void
TestInterpreterTestExpressionVerifier::testVerifyCompileError()
{
    ExpressionVerifier testee("testVerifyCompileError");
    TS_ASSERT_THROWS_NOTHING(testee.verifyCompileError("ByName(1)"));
    TS_ASSERT_THROWS(testee.verifyCompileError("("), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyCompileError("1"), AssertionFailedException);
}

void
TestInterpreterTestExpressionVerifier::testVerifyParseError()
{
    ExpressionVerifier testee("testVerifyParseError");
    TS_ASSERT_THROWS_NOTHING(testee.verifyParseError("("));
    TS_ASSERT_THROWS(testee.verifyParseError("1"), AssertionFailedException);
}

void
TestInterpreterTestExpressionVerifier::testVerifyStatement()
{
    ExpressionVerifier testee("testVerifyStatement");
    TS_ASSERT_THROWS_NOTHING(testee.verifyStatement("a:=1"));
    TS_ASSERT_THROWS(testee.verifyStatement("("), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyStatement("a:=b/c"), AssertionFailedException);   // divide by zero
}

