/**
  *  \file u/t_game_alliance.hpp
  *  \brief Tests for game::alliance
  */
#ifndef C2NG_U_T_GAME_ALLIANCE_HPP
#define C2NG_U_T_GAME_ALLIANCE_HPP

#include <cxxtest/TestSuite.h>

class TestGameAllianceContainer : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testIt();
    void testCopy();
    void testMerge();
    void testListener();
};

class TestGameAllianceHandler : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameAllianceHostHandler : public CxxTest::TestSuite {
 public:
    void testIt();
    void testOld();
};

class TestGameAllianceLevel : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameAllianceOffer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIsOffer();
};

class TestGameAlliancePHostHandler : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEnemy();
};

#endif
