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

class TestGameVcrClassicBattle : public CxxTest::TestSuite {
 public:
    void testSample();
    void testPosition();
    void testPoints();
    void testPointsRange();
};

class TestGameVcrClassicDatabase : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testLoadEmpty();
    void testLoadPHost4();
    void testLoadPHost3();
    void testLoadPHost2();
    void testLoadHost();
    void testLoadCorr();
    void testLoadNu();
};

class TestGameVcrClassicEventListener : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameVcrClassicEventRecorder : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSwap();
};

class TestGameVcrClassicHostAlgorithm : public CxxTest::TestSuite {
 public:
    void testFirst();
    void testSecond();
    void testLast();
    void testDeadFire();
    void testTenth();
};

class TestGameVcrClassicMirroringEventListener : public CxxTest::TestSuite {
 public:
    void testIt();
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
    void testToken();
};

class TestGameVcrClassicStatusToken : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrClassicTypes : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrClassicUtils : public CxxTest::TestSuite {
 public:
    void testFormatBattleResult();
};

class TestGameVcrClassicVisualizer : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
