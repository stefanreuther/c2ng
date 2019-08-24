/**
  *  \file u/t_game_alliance.hpp
  *  \brief Tests for game::alliance
  */
#ifndef C2NG_U_T_GAME_ALLIANCE_HPP
#define C2NG_U_T_GAME_ALLIANCE_HPP

#include <cxxtest/TestSuite.h>

class TestGameAllianceHandler : public CxxTest::TestSuite {
 public:
    void testInterface();
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

#endif
