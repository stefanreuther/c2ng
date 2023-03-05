/**
  *  \file u/t_interpreter_expr.hpp
  *  \brief Tests for interpreter::expr
  */
#ifndef C2NG_U_T_INTERPRETER_EXPR_HPP
#define C2NG_U_T_INTERPRETER_EXPR_HPP

#include <cxxtest/TestSuite.h>

class TestInterpreterExprAssignmentNode : public CxxTest::TestSuite {
 public:
    void testValue();
    void testEffect();
    void testOther();
};

class TestInterpreterExprBinaryNode : public CxxTest::TestSuite {
 public:
    void testValue();
    void testEffect();
    void testOther();
};

class TestInterpreterExprBuiltinFunction : public CxxTest::TestSuite {
 public:
    void testTrig();
    void testAbs();
    void testAsc();
    void testBitOps();
    void testMinMax();
    void testChr();
    void testTypeChecks();
    void testExp();
    void testStrFind();
    void testSubstr();
    void testTrim();
    void testSqrt();
    void testRound();
    void testIf();
    void testStr();
    void testVal();
    void testZap();
    void testLen();
    void testStrMult();
    void testStrCase();
    void testAtom();
    void testEval();
    void testMisc();
    void testIteration();
    void testKey();
};

class TestInterpreterExprCaseNode : public CxxTest::TestSuite {
 public:
    void testValueYes();
    void testValueNo();
    void testConvertYes();
    void testConvertNo();
};

class TestInterpreterExprFunctionCallNode : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterExprIdentifierNode : public CxxTest::TestSuite {
 public:
    void testValue();
    void testStore();
    void testCondition();
    void testReadWrite();
};

class TestInterpreterExprIndirectCallNode : public CxxTest::TestSuite {
 public:
    void testValue();
    void testStore();
    void testCondition();
    void testReadWrite();
};

class TestInterpreterExprLiteralNode : public CxxTest::TestSuite {
 public:
    void testValue();
    void testOther();
};

class TestInterpreterExprLogicalNode : public CxxTest::TestSuite {
 public:
    void testValue();
    void testEffect();
    void testCondition();
    void testOther();
};

class TestInterpreterExprMemberNode : public CxxTest::TestSuite {
 public:
    void testValue();
    void testStore();
    void testCondition();
    void testReadWrite();
};

class TestInterpreterExprNode : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestInterpreterExprParser : public CxxTest::TestSuite {
 public:
    void testLiterals();
    void testSequence();
    void testAssignment();
    void testOr();
    void testAnd();
    void testXor();
    void testNot();
    void testComparison();
    void testConcat();
    void testAdd();
    void testSubtract();
    void testMultiply();
    void testDivide();
    void testIntegerDivide();
    void testNegation();
    void testPower();
    void testPrecedence();
    void testErrors();
};

class TestInterpreterExprRValueFunctionCallNode : public CxxTest::TestSuite {
 public:
    void testOther();
};

class TestInterpreterExprRValueNode : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterExprSequenceNode : public CxxTest::TestSuite {
 public:
    void testValue();
    void testEffect();
    void testCondition();
    void testOther();
};

class TestInterpreterExprUnaryNode : public CxxTest::TestSuite {
 public:
    void testValue();
    void testEffect();
    void testOther();
};

#endif
