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

class TestInterpreterBinaryOperation : public CxxTest::TestSuite {
 public:
    void testName();
};

class TestInterpreterBlobValue : public CxxTest::TestSuite {
 public:
    void testIt();
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

class TestInterpreterContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterError : public CxxTest::TestSuite {
 public:
    void testTrace();
    void testInstances();
};

class TestInterpreterKeywords : public CxxTest::TestSuite {
 public:
    void testEnum();
    void testLookup();
};

class TestInterpreterMutexList : public CxxTest::TestSuite {
 public:
    void testDestruction();
};

class TestInterpreterNameTable : public CxxTest::TestSuite {
 public:
    void testLookup();
};

class TestInterpreterOptimizer : public CxxTest::TestSuite {
 public:
    void testStoreDrop1();
    void testStoreDrop2();
    void testStoreDrop3();
    void testStoreDrop4();
    void testStoreDrop5();
    void testMergeDrop1();
    void testMergeDrop2();
    void testNullOp1();
    void testNullOp2();
    void testNullOp3();
    void testNullOp4();
    void testEraseUnusedLabels1();
    void testEraseUnusedLabels2();
    void testInvertJumps1();
    void testInvertJumps2();
    void testInvertJumps3();
    void testInvertJumps4();
    void testInvertJumps5();
    void testInvertJumps6();
    void testInvertJumps7();
    void testInvertJumps8();
    void testInvertJumps9();
    void testInvertJumps10();
    void testInvertJumps11();
    void testThreadJumps1();
    void testThreadJumps2();
    void testThreadJumps3();
    void testThreadJumps4();
    void testThreadJumps5();
    void testThreadJumps6();
    void testThreadJumps7();
    void testThreadJumps8();
    void testThreadJumps9();
    void testThreadJumps10();
    void testRemoveUnused1();
    void testRemoveUnused2();
    void testRemoveUnused3();
    void testMergeNegation1();
    void testMergeNegation2();
    void testMergeNegation3();
    void testMergeNegation4();
    void testMergeNegation5();
    void testMergeNegation6();
    void testMergeNegation7();
    void testMergeNegation8();
    void testMergeNegation9();
    void testUnaryCondition1();
    void testUnaryCondition2();
    void testUnaryCondition3();
    void testUnaryCondition4();
    void testUnaryCondition5();
    void testFoldUnary1();
    void testFoldUnary2();
    void testFoldUnary3();
    void testFoldUnary4();
    void testFoldBinaryInt1();
    void testFoldBinaryInt2();
    void testFoldBinaryInt3();
    void testFoldBinaryInt4();
    void testFoldJump1();
    void testFoldJump2();
    void testFoldJump3();
    void testFoldJump4();
    void testPopPush1();
    void testPopPush2();
    void testPopPush3();
    void testCompareNC1();
    void testCompareNC2();
    void testCompareNC3();
    void testCompareNC4();
    void testFailAbsolute1();
    void testFailAbsolute2();
    void testFailFoldUnary();
    void testFailFoldBinary();
};

class TestInterpreterProcedureValue : public CxxTest::TestSuite {
 public:
    void testIt();
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

class TestInterpreterSelectionExpression : public CxxTest::TestSuite {
 public:
    void testValid();
    void testInvalid();
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

class TestInterpreterTernaryOperation : public CxxTest::TestSuite {
 public:
    void testName();
};

class TestInterpreterTokenizer : public CxxTest::TestSuite {
 public:
    void testTokenizer();
    void testIntegers();
    void testFloats();
    void testStrings();
};

class TestInterpreterUnaryOperation : public CxxTest::TestSuite {
 public:
    void testName();
};

class TestInterpreterValues : public CxxTest::TestSuite {
 public:
    void testStringToString();
    void testOtherToString();
    void testIntToString();
    void testBoolToString();
    void testFloatToString();
    void testMiscToString();
};

#endif
