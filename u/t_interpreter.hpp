/**
  *  \file u/t_interpreter.hpp
  *  \brief Tests for interpreter
  */
#ifndef C2NG_U_T_INTERPRETER_HPP
#define C2NG_U_T_INTERPRETER_HPP

#include <cxxtest/TestSuite.h>
#include "afl/base/types.hpp"

class ExpressionTestHelper {
 public:
    class TestContext;
    friend class TestContext;

    ExpressionTestHelper();

    void checkIntegerExpression(const char* expr, int result);
    void checkBooleanExpression(const char* expr, int result);
    void checkFileExpression(const char* expr, int result);
    void checkNullExpression(const char* expr);
    void checkStringExpression(const char* expr, const char* result);
    void checkFloatExpression(const char* expr, double result);
    void checkFailureExpression(const char* expr);

    void checkStatement(const char* stmt);
    void checkIntegerExpressionStatement(const char* expr, int value);

    int32_t a, b, c;

 private:
    void checkScalarExpression(const char* expr, int result, bool isBool);
};

class TestInterpreterBytecodeObject : public CxxTest::TestSuite {
 public:
    void testGet();
    void testArgs();
    void testCopyLocalVariablesFrom();
    void testLabel();
};

class TestInterpreterClosure : public CxxTest::TestSuite {
 public:
    void testClosure();
};

class TestInterpreterKeywords : public CxxTest::TestSuite {
 public:
    void testEnum();
    void testLookup();
};

class TestInterpreterNameTable : public CxxTest::TestSuite {
 public:
    void testLookup();
};

class TestInterpreterProcess : public CxxTest::TestSuite {
 public:
    void testProperties();
};

class TestInterpreterProcessList : public CxxTest::TestSuite {
 public:
    void testEmpty1();
    void testEmpty2();
    void testAllocateProcessGroup();
    void testSuspend();
};

class TestInterpreterStatementCompiler : public CxxTest::TestSuite {
 public:
    void testExprStatement();
    void testIf();
    void testFor();
    void testDo();
    void testSelect();
    void testEval();
};

class TestInterpreterTokenizer : public CxxTest::TestSuite {
 public:
    void testTokenizer();
    void testIntegers();
    void testFloats();
    void testStrings();
};

class TestInterpreterValues : public CxxTest::TestSuite {
 public:
    void testStringToString();
};

#endif
