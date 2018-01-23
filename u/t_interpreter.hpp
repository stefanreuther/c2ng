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
    void checkBadExpression(const char* expr);

    void checkStatement(const char* stmt);
    void checkIntegerExpressionStatement(const char* expr, int value);

    int32_t a, b, c;

 private:
    void checkScalarExpression(const char* expr, int result, bool isBool);
};

class TestInterpreterArguments : public CxxTest::TestSuite {
 public:
    void testIt();
    void testArgumentCount();
    void testInteger();
    void testBoolean();
    void testString();
    void testFlagArg();
    void testAtomArg();
};

class TestInterpreterArrayData : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testMatrix();
    void testResize();
    void testDimension();
};

class TestInterpreterArrayValue : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterBaseValue : public CxxTest::TestSuite {
 public:
    void testIt();
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
    void testLiteral();
    void testIntLiteral();
    void testMakeLabelOverflow();
    void testMakeLabelOverflow2();
    void testAddNameOverflow();
    void testNames();
    void testLineNumbers();
    void testLineNumbers2();
    void testLineNumbers3();
    void testHasUserCall();
};

class TestInterpreterCallableValue : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestInterpreterClosure : public CxxTest::TestSuite {
 public:
    void testClosure();
};

class TestInterpreterCommandSource : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterCompilationContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterContextProvider : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestInterpreterDefaultStatementCompilationContext : public CxxTest::TestSuite {
 public:
    void testStandalone();
    void testParented();
};

class TestInterpreterError : public CxxTest::TestSuite {
 public:
    void testTrace();
    void testInstances();
};

class TestInterpreterFileCommandSource : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterFileFunctions : public CxxTest::TestSuite {
 public:
    void testSet();
    void testPositionFunctions();
};

class TestInterpreterFileTable : public CxxTest::TestSuite {
 public:
    void testIt();
    void testAppend();
};

class TestInterpreterFileValue : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterFusion : public CxxTest::TestSuite {
 public:
    void testFusedBinary();
    void testFusedUnary();
    void testInplaceUnary();
    void testFusedComparison();
    void testMisc();
};

class TestInterpreterHashValue : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testUnit();
    void testMulti();
};

class TestInterpreterKeymapValue : public CxxTest::TestSuite {
 public:
    void testIt();
    void testMake();
};

class TestInterpreterKeywords : public CxxTest::TestSuite {
 public:
    void testEnum();
    void testLookup();
};

class TestInterpreterMemoryCommandSource : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterMutexList : public CxxTest::TestSuite {
 public:
    void testDestruction();
    void testIt();
    void testAbandon();
    void testDisown();
};

class TestInterpreterNameTable : public CxxTest::TestSuite {
 public:
    void testLookup();
};

class TestInterpreterObjectPropertyVector : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterOpcode : public CxxTest::TestSuite {
 public:
    void testPush();
    void testBinary();
    void testUnary();
    void testTernary();
    void testJump();
    void testIndirect();
    void testStack();
    void testPop();
    void testStore();
    void testMemref();
    void testDim();
    void testSpecial();
    void testFusedUnary();
    void testFusedBinary();
    void testFusedComparison();
    void testFusedComparison2();
    void testInplaceUnary();
    void testUnknown();
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
    void testJoin();
    void testFail();
    void testTerminate();
    void testPriority();
};

class TestInterpreterPropertyAcceptor : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterSaveContext : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestInterpreterSelectionExpression : public CxxTest::TestSuite {
 public:
    void testValid();
    void testInvalid();
};

class TestInterpreterSimpleSpecialCommand : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterSpecialCommand : public CxxTest::TestSuite {
 public:
    void testInterface();
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

class TestInterpreterStructureType : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterStructureTypeData : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterStructureValue : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterStructureValueData : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterTagNode : public CxxTest::TestSuite {
 public:
    void testHeader();
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
    void testIsIdentifierCharacter();
    void testIsValidUppercaseIdentifier();
};

class TestInterpreterTypeHint : public CxxTest::TestSuite {
 public:
    void testHeader();
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
    void testMake();
};

class TestInterpreterWorld : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSpecial();
    void testLoad();
};

#endif
