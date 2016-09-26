/**
  *  \file u/t_game_spec.hpp
  */
#ifndef C2NG_U_T_GAME_SPEC_HPP
#define C2NG_U_T_GAME_SPEC_HPP

#include <cxxtest/TestSuite.h> 

class TestGameSpecComponent : public CxxTest::TestSuite {
 public:
    void testData();
    void testName();
};

class TestGameSpecComponentNameProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSpecComponentVector : public CxxTest::TestSuite {
 public:
    void testCreate();
    void testName();
};

class TestGameSpecCost : public CxxTest::TestSuite {
 public:
    void testParse();
    void testAdd();             // +=
    void testSubtract();        // -=
    void testMult();            // *=, *
    void testCompare();         // ==, !=
    void testEnough();          // isEnoughFor()
};

class TestGameSpecEngine : public CxxTest::TestSuite {
 public:
    void testFuelDefaults();
    void testFuel();
};

class TestGameSpecFriendlyCode : public CxxTest::TestSuite {
 public:
    void testFCode();
};

class TestGameSpecFriendlyCodeList : public CxxTest::TestSuite {
 public:
    void testNumeric();
    void testRandom();
};

class TestGameSpecHullFunctionList : public CxxTest::TestSuite {
 public:
    void testSimplify();
};

class TestGameSpecMissionList : public CxxTest::TestSuite {
 public:
    void testMissionIni();
    void testMissionIniRaces();
};

#endif
