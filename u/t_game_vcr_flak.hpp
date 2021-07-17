/**
  *  \file u/t_game_vcr_flak.hpp
  *  \brief Tests for game::vcr::flak
  */
#ifndef C2NG_U_T_GAME_VCR_FLAK_HPP
#define C2NG_U_T_GAME_VCR_FLAK_HPP

#include <cxxtest/TestSuite.h>

class TestGameVcrFlakAlgorithm : public CxxTest::TestSuite {
 public:
    void testPlay();
    void testPlayNonAC();
    void testSetup();
    void testSetupFighters();
    void testCloneStatus();
    void testSetupCapture();
    void testSetupCaptureDeathRay();
    void testPair();
};

class TestGameVcrFlakBattle : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrFlakConfiguration : public CxxTest::TestSuite {
 public:
    void testInit();
    void testLoad();
};

class TestGameVcrFlakDatabase : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrFlakEnvironment : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameVcrFlakEventRecorder : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSwap();
};

class TestGameVcrFlakGameEnvironment : public CxxTest::TestSuite {
 public:
    void testConfig();
    void testSpec();
};

class TestGameVcrFlakObject : public CxxTest::TestSuite {
 public:
    void testIt();
    void testPack();
};

class TestGameVcrFlakPosition : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrFlakSetup : public CxxTest::TestSuite {
 public:
    void testInit();
    void testIO();
};

class TestGameVcrFlakStructures : public CxxTest::TestSuite {
 public:
    void testInclude();
};

class TestGameVcrFlakVisualisationSettings : public CxxTest::TestSuite {
 public:
    void testInit();
    void testMove();
    void testFollowFleet();
    void testFollowPlayer();
    void testFollowError();
    void testRaise();
    void testZoom();
    void testFollowDead();
    void testFollowDead2();
};

class TestGameVcrFlakVisualisationState : public CxxTest::TestSuite {
 public:
    void testInit();
    void testBounds();
    void testShip();
    void testFleet();
    void testFighter();
    void testTorpedo();
    void testBeam();
    void testSmokeAge();
    void testBeamAge();
    void testShipTurn();
    void testCopy();
    void testTime();
};

#endif
