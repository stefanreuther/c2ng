/**
  *  \file u/t_game_vcr.hpp
  *  \brief Tests for game::vcr
  */
#ifndef C2NG_U_T_GAME_VCR_HPP
#define C2NG_U_T_GAME_VCR_HPP

#include <cxxtest/TestSuite.h>

class TestGameVcrBattle : public CxxTest::TestSuite {
 public:
    void testIt();
    void testDescription();
};

class TestGameVcrDatabase : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrInfo : public CxxTest::TestSuite {
 public:
    void testInit();
};

class TestGameVcrNullDatabase : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrObject : public CxxTest::TestSuite {
 public:
    void testGetSet();
    void testAdd();
    void testGuess();
    void testGuessAmbig();
    void testGuessMismatch();
    void testGuessEngine();
    void testDescribe();
};

class TestGameVcrObjectInfo : public CxxTest::TestSuite {
 public:
    void testPlanet1();
    void testPlanet2();
    void testPlanet3();
    void testPlanet4();
    void testFailPlanet1();
    void testFailPlanet2();
    void testFailNotPlanet();
    void testShip();
    void testShip2();
};

class TestGameVcrOverview : public CxxTest::TestSuite {
 public:
    void testDiagram();
    void testDiagramKill();
    void testDiagramStalemate();
};

class TestGameVcrScore : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrStatistic : public CxxTest::TestSuite {
 public:
    void testInit();
    void testIt();
};

#endif
