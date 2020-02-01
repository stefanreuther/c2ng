/**
  *  \file u/t_game_map_minefield.cpp
  *  \brief Test for game::map::Minefield
  *
  *  Test cases have been obtained using c2hosttest/mine/01_decay.
  */

#include "game/map/minefield.hpp"

#include "t_game_map.hpp"

/** Test mine decay, THost version. */
void
TestGameMapMinefield::testUnitsAfterDecayHost()
{
    // Environment
    game::HostVersion host(game::HostVersion::Host, MKVERSION(3, 22, 46));
    game::config::HostConfiguration config;
    config[config.MineDecayRate].set(5);

    // Minefield
    game::map::Minefield testee(7, game::map::Point(1000, 1000), 1, false, 200);

    // Test cases
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay(  5, host, config),  4);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 10, host, config),  9);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 15, host, config), 13);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 20, host, config), 18);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 25, host, config), 23);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 30, host, config), 27);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 35, host, config), 32);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 40, host, config), 37);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 45, host, config), 42);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 50, host, config), 47);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 55, host, config), 51);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 60, host, config), 56);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 65, host, config), 61);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 70, host, config), 65);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 75, host, config), 70);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 80, host, config), 75);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 85, host, config), 80);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 90, host, config), 85);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 95, host, config), 89);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay(100, host, config), 94);
}

/** Test mine decay, Post version. */
void
TestGameMapMinefield::testUnitsAfterDecayPHost()
{
    // Environment
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(4, 0, 0));
    game::config::HostConfiguration config;
    config[config.MineDecayRate].set(5);

    // Minefield
    game::map::Minefield testee(7, game::map::Point(1000, 1000), 1, false, 200);

    // Test cases
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay(  5, host, config),  4);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 10, host, config),  9);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 15, host, config), 14);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 20, host, config), 19);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 25, host, config), 23);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 30, host, config), 28);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 35, host, config), 33);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 40, host, config), 38);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 45, host, config), 42);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 50, host, config), 47);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 55, host, config), 52);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 60, host, config), 57);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 65, host, config), 61);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 70, host, config), 66);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 75, host, config), 71);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 80, host, config), 76);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 85, host, config), 80);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 90, host, config), 85);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 95, host, config), 90);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay(100, host, config), 95);
}

