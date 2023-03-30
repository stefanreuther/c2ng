/**
  *  \file u/t_interpreter_test.hpp
  *  \brief Tests for interpreter::test
  */
#ifndef C2NG_U_T_INTERPRETER_TEST_HPP
#define C2NG_U_T_INTERPRETER_TEST_HPP

#include <cxxtest/TestSuite.h>

class TestInterpreterTestContextVerifier : public CxxTest::TestSuite {
 public:
    void testVerifyTypesSuccess();
    void testVerifyTypesDuplicate();
    void testVerifyTypesMismatch();
    void testVerifyTypesTypeCheck();
    void testVerifyTypesNull();
    void testVerifyInteger();
    void testVerifyBoolean();
    void testVerifyString();
    void testVerifyNull();
    void testSet();
};

class TestInterpreterTestExpressionVerifier : public CxxTest::TestSuite {
 public:
    void testInit();
    void testVerifyInteger();
    void testVerifyBoolean();
    void testVerifyFile();
    void testVerifyNull();
    void testVerifyString();
    void testVerifyFloat();
    void testVerifyExecutionError();
    void testVerifyCompileError();
    void testVerifyParseError();
    void testVerifyStatement();
};

class TestInterpreterTestValueVerifier : public CxxTest::TestSuite {
 public:
    void testFailBasicString();
    void testFailBasicClone();
    void testFailBasicClonedString();
    void testFailSerializeGood();
    void testFailSerializeBad();
    void testFailSerializeFail();
    void testVerifyNewInteger();
    void testVerifyNewFloat();
    void testVerifyNewBoolean();
    void testVerifyNewString();
    void testVerifyNewNull();
};

#endif
