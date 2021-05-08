/**
  *  \file u/t_game_spec_hullassignmentlist.cpp
  *  \brief Test for game::spec::HullAssignmentList
  */

#include "game/spec/hullassignmentlist.hpp"

#include "t_game_spec.hpp"
#include "game/config/hostconfiguration.hpp"

/** Simple test. */
void
TestGameSpecHullAssignmentList::testIt()
{
    // A configuration
    game::config::HostConfiguration config;
    config[config.MapTruehullByPlayerRace].set(false);

    // Configure a testee
    game::spec::HullAssignmentList testee;
    for (int player = 1; player <= 5; ++player) {
        for (int slot = 1; slot <= 10; ++slot) {
            testee.add(player, slot, 100*player+slot);
        }
    }

    // Out-of-range access (all ignored)
    testee.add(1, 1, 0);
    testee.add(1, 0, 1);
    testee.add(0, 1, 1);

    testee.add(1, 1, -1);
    testee.add(1, -1, 1);
    testee.add(-1, 1, 1);

    // Verify access
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 1, 1), 101);
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 2, 2), 202);
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 5, 10), 510);

    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 0, 0), 0);
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, -1, -1), 0);
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 6, 6), 0);

    TS_ASSERT_EQUALS(testee.getMaxIndex(config, 0), 0);
    TS_ASSERT_EQUALS(testee.getMaxIndex(config, 1), 10);
    TS_ASSERT_EQUALS(testee.getMaxIndex(config, 5), 10);
    TS_ASSERT_EQUALS(testee.getMaxIndex(config, 6), 0);

    TS_ASSERT_EQUALS(testee.getIndexFromHull(config, 1, 107), 7);
    TS_ASSERT_EQUALS(testee.getIndexFromHull(config, 1, 111), 0);
    TS_ASSERT_EQUALS(testee.getIndexFromHull(config, 1, 201), 0);
    TS_ASSERT_EQUALS(testee.getIndexFromHull(config, 2, 201), 1);

    TS_ASSERT_EQUALS(testee.getPlayersForHull(config, 107), game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(testee.getPlayersForHull(config, 201), game::PlayerSet_t(2));
    TS_ASSERT_EQUALS(testee.getPlayersForHull(config, 501), game::PlayerSet_t(5));
    TS_ASSERT_EQUALS(testee.getPlayersForHull(config, 999), game::PlayerSet_t());

    // Selective clear
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 3, 5), 305);
    TS_ASSERT_EQUALS(testee.getIndexFromHull(config, 3, 305), 5);
    testee.clearPlayer(3);
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 3, 5), 0);
    TS_ASSERT_EQUALS(testee.getIndexFromHull(config, 3, 305), 0);

    // Full clear
    testee.clear();
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 1, 1), 0);
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 2, 2), 0);
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 5, 10), 0);
}

/** Test PlayerRace. */
void
TestGameSpecHullAssignmentList::testPlayerRace()
{
    // A configuration
    game::config::HostConfiguration config;
    config[config.MapTruehullByPlayerRace].set(true);
    config[config.PlayerRace].set("6,5,4,3,2,1");

    // Configure a testee
    game::spec::HullAssignmentList testee;
    for (int player = 1; player <= 5; ++player) {
        for (int slot = 1; slot <= 10; ++slot) {
            testee.add(player, slot, 100*player+slot);
        }
    }

    // Default
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 1, 5), 105);
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 2, 5), 205);

    // Reconfigure
    testee.setMode(testee.RaceIndexed);
    
    // Default
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 1, 5), 0);
    TS_ASSERT_EQUALS(testee.getHullFromIndex(config, 2, 5), 505);
    TS_ASSERT_EQUALS(testee.getIndexFromHull(config, 1, 505), 0);
    TS_ASSERT_EQUALS(testee.getIndexFromHull(config, 2, 505), 5);
    TS_ASSERT_EQUALS(testee.getMaxIndex(config, 1), 0);
    TS_ASSERT_EQUALS(testee.getMaxIndex(config, 2), 10);
}

