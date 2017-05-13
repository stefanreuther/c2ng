/**
  *  \file u/t_game_actions.hpp
  *  \brief Tests for game::actions
  */
#ifndef C2NG_U_T_GAME_ACTIONS_HPP
#define C2NG_U_T_GAME_ACTIONS_HPP

#include <cxxtest/TestSuite.h>

class TestGameActionsPreconditions : public CxxTest::TestSuite {
 public:
    void testShip();
    void testPlanet();
    void testBase();
    void testSession();
};

#endif
