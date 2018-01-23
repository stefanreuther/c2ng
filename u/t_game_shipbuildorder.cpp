/**
  *  \file u/t_game_shipbuildorder.cpp
  *  \brief Test for game::ShipBuildOrder
  */

#include "game/shipbuildorder.hpp"

#include "t_game.hpp"

/** Test data members. */
void
TestGameShipBuildOrder::testIt()
{
    // Test initial state
    game::ShipBuildOrder testee;
    TS_ASSERT_EQUALS(testee.getHullIndex(), 0);
    TS_ASSERT_EQUALS(testee.getEngineType(), 0);
    TS_ASSERT_EQUALS(testee.getBeamType(), 0);
    TS_ASSERT_EQUALS(testee.getNumBeams(), 0);
    TS_ASSERT_EQUALS(testee.getLauncherType(), 0);
    TS_ASSERT_EQUALS(testee.getNumLaunchers(), 0);

    // Configure
    testee.setHullIndex(15);
    testee.setEngineType(9);
    testee.setBeamType(3);
    testee.setNumBeams(18);
    testee.setLauncherType(8);
    testee.setNumLaunchers(4);

    // Verify
    TS_ASSERT_EQUALS(testee.getHullIndex(), 15);
    TS_ASSERT_EQUALS(testee.getEngineType(), 9);
    TS_ASSERT_EQUALS(testee.getBeamType(), 3);
    TS_ASSERT_EQUALS(testee.getNumBeams(), 18);
    TS_ASSERT_EQUALS(testee.getLauncherType(), 8);
    TS_ASSERT_EQUALS(testee.getNumLaunchers(), 4);
}

/** Test comparison. */
void
TestGameShipBuildOrder::testComparison()
{
    // Default
    TS_ASSERT_EQUALS(game::ShipBuildOrder() == game::ShipBuildOrder(), true);
    TS_ASSERT_EQUALS(game::ShipBuildOrder() != game::ShipBuildOrder(), false);

    // Nondefault
    game::ShipBuildOrder testee;
    testee.setHullIndex(15);
    testee.setEngineType(9);
    testee.setBeamType(3);
    testee.setNumBeams(18);
    testee.setLauncherType(8);
    testee.setNumLaunchers(4);
    TS_ASSERT_EQUALS(testee == testee, true);
    TS_ASSERT_EQUALS(testee == game::ShipBuildOrder(), false);
}

/** Test canonicalize(). */
void
TestGameShipBuildOrder::testCanonicalize()
{
    game::ShipBuildOrder testee;
    testee.setHullIndex(15);
    testee.setEngineType(9);
    testee.setBeamType(3);
    testee.setNumBeams(0);
    testee.setLauncherType(8);
    testee.setNumLaunchers(0);
    testee.canonicalize();

    TS_ASSERT_EQUALS(testee.getLauncherType(), 0);
    TS_ASSERT_EQUALS(testee.getBeamType(), 0);
}

