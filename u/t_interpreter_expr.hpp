/**
  *  \file u/t_interpreter_expr.hpp
  *  \brief Tests for interpreter::expr
  */
#ifndef C2NG_U_T_INTERPRETER_EXPR_HPP
#define C2NG_U_T_INTERPRETER_EXPR_HPP

#include <cxxtest/TestSuite.h>

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

#endif
