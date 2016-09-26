/**
  *  \file u/t_game_vcr_classic.hpp
  *  \brief Tests for game::vcr::classic
  */
#ifndef C2NG_U_T_GAME_VCR_CLASSIC_HPP
#define C2NG_U_T_GAME_VCR_CLASSIC_HPP

#include <cxxtest/TestSuite.h>

class TestGameVcrClassicAlgorithm : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrClassicHostAlgorithm : public CxxTest::TestSuite {
 public:
    void testFirst();
    void testSecond();
    void testLast();
    void testDeadFire();
};

class TestGameVcrClassicNullVisualizer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrClassicPVCRAlgorithm : public CxxTest::TestSuite {
 public:
    void testTF();
    void testCarriers();
    void testRandomBonus();
};

class TestGameVcrClassicStatusToken : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrClassicVisualizer : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
