/**
  *  \file u/t_game_shipbuildorder.cpp
  *  \brief Test for game::ShipBuildOrder
  */

#include "game/shipbuildorder.hpp"

#include "t_game.hpp"
#include "game/test/shiplist.hpp"
#include "afl/string/nulltranslator.hpp"

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
    TS_ASSERT_EQUALS(testee.toScriptCommand("Make", 0), "Make 15, 9, 3, 18, 8, 4");
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

void
TestGameShipBuildOrder::testDescribe()
{
    game::spec::ShipList sl;
    game::test::initStandardBeams(sl);
    game::test::initStandardTorpedoes(sl);
    game::test::addNovaDrive(sl);
    game::test::addAnnihilation(sl);
    sl.hulls().get(game::test::ANNIHILATION_HULL_ID)->setShortName("Annihilation");

    game::ShipBuildOrder testee;
    testee.setHullIndex(game::test::ANNIHILATION_HULL_ID);
    testee.setEngineType(5);
    testee.setBeamType(3);
    testee.setNumBeams(4);
    testee.setLauncherType(8);
    testee.setNumLaunchers(1);

    afl::data::StringList_t result;
    afl::string::NullTranslator tx;
    testee.describe(result, sl, tx);

    TS_ASSERT_EQUALS(result.size(), 4U);
    TS_ASSERT_EQUALS(result[0], "ANNIHILATION CLASS BATTLESHIP");
    TS_ASSERT_EQUALS(result[1], "6 \xC3\x97 Nova Drive 5");
    TS_ASSERT_EQUALS(result[2], "4 \xC3\x97 Plasma Bolt");
    TS_ASSERT_EQUALS(result[3], "Mark 6 Photon");
    TS_ASSERT_EQUALS(testee.toScriptCommand("Make", &sl), "Make 53, 5, 3, 4, 8, 1   % Annihilation");
}

void
TestGameShipBuildOrder::testDescribeCarrier()
{
    game::spec::ShipList sl;
    game::test::initStandardBeams(sl);
    game::test::initStandardTorpedoes(sl);
    game::test::addTranswarp(sl);
    game::test::addGorbie(sl);

    game::ShipBuildOrder testee;
    testee.setHullIndex(game::test::GORBIE_HULL_ID);
    testee.setEngineType(9);
    testee.setBeamType(10);
    testee.setNumBeams(1);
    testee.setLauncherType(10);
    testee.setNumLaunchers(0);

    afl::data::StringList_t result;
    afl::string::NullTranslator tx;
    testee.describe(result, sl, tx);

    TS_ASSERT_EQUALS(result.size(), 4U);
    TS_ASSERT_EQUALS(result[0], "GORBIE CLASS BATTLECARRIER");
    TS_ASSERT_EQUALS(result[1], "6 \xC3\x97 Transwarp Drive");
    TS_ASSERT_EQUALS(result[2], "Heavy Phaser");
    TS_ASSERT_EQUALS(result[3], "10 fighter bays");
}

