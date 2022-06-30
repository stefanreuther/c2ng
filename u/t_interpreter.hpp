/**
  *  \file u/t_interpreter.hpp
  *  \brief Tests for interpreter
  */
#ifndef C2NG_U_T_INTERPRETER_HPP
#define C2NG_U_T_INTERPRETER_HPP

#include <cxxtest/TestSuite.h>
#include "afl/base/types.hpp"

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

class TestInterpreterBinaryExecution : public CxxTest::TestSuite {
 public:
    void testAnd();
    void testOr();
    void testXor();
    void testAdd();
    void testSub();
    void testMult();
    void testDivide();
    void testIntegerDivide();
    void testRemainder();
    void testPow();
    void testConcat();
    void testConcatEmpty();
    void testCompare();
    void testMin();
    void testMax();
    void testFirstStr();
    void testRestStr();
    void testFindStr();
    void testBitAnd();
    void testBitOr();
    void testBitXor();
    void testStr();
    void testATan();
    void testLCut();
    void testRCut();
    void testEndCut();
    void testStrMult();
    void testKeyAddParent();
    void testKeyFind();
    void testArrayDim();
    void testExecuteComparison();
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
    void testVariableReference();
    void testCompact();
    void testAppend();
    void testDisassembly();
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
    void testPropertyAccessor();
    void testReadOnlyAccessor();
};

class TestInterpreterContextReceiver : public CxxTest::TestSuite {
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

class TestInterpreterFunctionValue : public CxxTest::TestSuite {
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

class TestInterpreterGenericValue : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterHashValue : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testUnit();
    void testMulti();
};

class TestInterpreterIndexableValue : public CxxTest::TestSuite {
 public:
    void testIt();
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
    void testOneLine();
    void testTwoLines();
    void testCharset();
    void testError();
    void testAddLinesEmpty();
    void testAddLinesOne();
    void testAddLinesMulti();
};

class TestInterpreterMutexContext : public CxxTest::TestSuite {
 public:
    void testSave();
    void testBasics();
};

class TestInterpreterMutexFunctions : public CxxTest::TestSuite {
 public:
    void testInit();
    void testTakeLock();
    void testTakeLockConflict();
    void testReleaseLockOnExit();
    void testGetLockInfo0();
    void testGetLockInfo1();
    void testGetLockInfo2();
    void testGetLockInfoFail0();
    void testGetLockInfoFail1();
    void testGetLockInfoFail2();
    void testFailNull();
    void testFailIter();
    void testDim();
    void testToString();
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
    void testIntCompare();
    void testTailMerge();
    void testLabelFailure1();
    void testLabelFailure2();
    void testLabelFailure3();
    void testLabelFailure4();
    void testLabelFailure5();
    void testDeadStore1();
    void testDeadStore2();
    void testDeadStore3();
};

class TestInterpreterProcedureValue : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterProcess : public CxxTest::TestSuite {
 public:
    void testProperties();
    void testFreeze();
    void testFreeze2();
    void testFinalize();
    void testFinalize2();
    void testContextStack();
    void testContextStack2();
    void testVariable();
    void testExecInvalid();
    void testExecPushNamed();
    void testExecPushLocal();
    void testExecPushStatic();
    void testExecPushShared();
    void testExecPushNamedShared();
    void testExecPushLiteral();
    void testExecPushInteger();
    void testExecPushBoolean();
    void testExecUnary();
    void testExecBinary();
    void testExecTernary();
    void testExecJumpCondTaken();
    void testExecJumpCondMiss();
    void testExecJumpAlways();
    void testExecJumpCatch();
    void testExecJumpDecZero();
    void testExecIndirectCall();
    void testExecIndirectLoad();
    void testExecIndirectStore();
    void testExecIndirectPop();
    void testExecStackDup();
    void testExecStackDrop();
    void testExecStackSwap();
    void testExecStoreNamedVariable();
    void testExecStoreLocal();
    void testExecStoreStatic();
    void testExecStoreShared();
    void testExecStoreNamedShared();
    void testExecPopNamedVariable();
    void testExecPopLocal();
    void testExecPopStatic();
    void testExecPopShared();
    void testExecPopNamedShared();
    void testExecMemrefLoad();
    void testExecMemrefCall();
    void testExecMemrefPop();
    void testExecMemrefStore();
    void testExecDimLocal();
    void testExecDimStatic();
    void testExecDimShared();
    void testExecUncatch();
    void testExecReturn();
    void testExecWith();
    void testExecEndWith();
    void testExecFirstIndex();
    void testExecNextIndex();
    void testExecEndIndex();
    void testExecEvalStatement();
    void testExecEvalExpression();
    void testExecDefSub();
    void testExecDefShipProperty();
    void testExecDefPlanetProperty();
    void testExecLoad();
    void testExecPrint();
    void testExecAddHook();
    void testExecRunHook();
    void testExecThrow();
    void testExecTerminate();
    void testExecSuspend();
    void testExecNewArray();
    void testExecMakeList();
    void testExecNewHash();
    void testExecInstance();
    void testExecResizeArray();
    void testExecBind();
    void testExecFirst();
    void testExecNext();
    void testExecFusedUnary();
    void testExecFusedBinary();
    void testExecFusedComparison();
    void testExecFusedComparison2();
    void testExecInplaceUnary();
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
    void testEndSignal();
    void testWait();
    void testWaitError();
    void testWaitCatch();
    void testWaitTerminate();
    void testRemoveKeep();
    void testResume();
    void testTerminateAll();
    void testResumeNone();
    void testMismatches();
    void testRunFreeze();
    void testObject();
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

class TestInterpreterSimpleFunction : public CxxTest::TestSuite {
 public:
    void testValue();
    void testVoid();
};

class TestInterpreterSimpleProcedure : public CxxTest::TestSuite {
 public:
    void testValue();
    void testVoid();
};

class TestInterpreterSimpleSpecialCommand : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterSpecialCommand : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestInterpreterStatementCompilationContext : public CxxTest::TestSuite {
 public:
    void testConstructor();
    void testDefaults();
    void testOneLineSyntax();
    void testBlockSyntax();
};

class TestInterpreterStatementCompiler : public CxxTest::TestSuite {
 public:
    void testExprStatement();
    void testBreakFailures();
    void testMisplaced();
    void testIf();
    void testFor();
    void testDo();
    void testSelect();
    void testEval();
    void testEnd();
    void testStop();
    void testAbort();
    void testSub();
    void testFunction();
    void testCreateProperty();
    void testDimLocal();
    void testDimStatic();
    void testDimShared();
    void testBind();
    void testCreateKeymap();
    void testForEach();
    void testHooks();
    void testUseKeymap();
    void testSelectionExec();
    void testStructWith();
    void testTry();
    void testRedim();
    void testLoad();
    void testPreexecLoad();
    void testPrint();
    void testOption();
    void testCompileList();
    void testSpecial();
};

class TestInterpreterStaticContext : public CxxTest::TestSuite {
 public:
    void testInterface();
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

class TestInterpreterSubroutineValue : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterTagNode : public CxxTest::TestSuite {
 public:
    void testHeader();
};

class TestInterpreterTaskEditor : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testAddToEmpty();
    void testRoundtrip();
    void testConflict();
    void testFormat();
    void testIsValidCommand();
};

class TestInterpreterTaskPredictor : public CxxTest::TestSuite {
 public:
    void testPredictTask();
    void testPredictRestart();
    void testPredictRestart2();
    void testPredictError();
    void testPredictStatement();
    void testPredictStatementError();
};

class TestInterpreterTernaryExecution : public CxxTest::TestSuite {
 public:
    void testKeyAdd();
    void testInvalid();
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
    void testBadStrings();
};

class TestInterpreterTypeHint : public CxxTest::TestSuite {
 public:
    void testHeader();
};

class TestInterpreterUnaryExecution : public CxxTest::TestSuite {
 public:
    void testInvalid();
    void testNot();
    void testBool();
    void testNeg();
    void testPos();
    void testSin();
    void testCos();
    void testTan();
    void testZap();
    void testAbs();
    void testExp();
    void testLog();
    void testBitNot();
    void testIsEmpty();
    void testIsNum();
    void testIsString();
    void testAsc();
    void testChr();
    void testStr();
    void testSqrt();
    void testTrunc();
    void testRound();
    void testLTrim();
    void testRTrim();
    void testLRTrim();
    void testLength();
    void testVal();
    void testTrace();
    void testNot2();
    void testAtom();
    void testAtomStr();
    void testKeyCreate();
    void testKeyLookup();
    void testInc();
    void testDec();
    void testIsProcedure();
    void testFileNr();
    void testIsArray();
    void testUCase();
    void testLCase();
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
    void testQuoteString();
    void testQuoteStringParse();
    void testFormatFloat();
};

class TestInterpreterWorld : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSpecial();
    void testLoad();
};

#endif
