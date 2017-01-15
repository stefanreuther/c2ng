/**
  *  \file u/t_game_sim.hpp
  *  \brief Tests for game::sim
  */
#ifndef C2NG_U_T_GAME_SIM_HPP
#define C2NG_U_T_GAME_SIM_HPP

#include <cxxtest/TestSuite.h>
#include "game/sim/object.hpp"

class TestGameSimLoader : public CxxTest::TestSuite {
 public:
    void testV0();
    void testV1();
    void testV2();
    void testV3();
    void testV4();
    void testV5();
    void testError();
};

class TestGameSimObject : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSetRandom();
    void testRandom();

    static void verifyObject(game::sim::Object& t);
};

class TestGameSimPlanet : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSimShip : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
