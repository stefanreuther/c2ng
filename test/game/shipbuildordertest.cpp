/**
  *  \file test/game/shipbuildordertest.cpp
  *  \brief Test for game::ShipBuildOrder
  */

#include "game/shipbuildorder.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/shiplist.hpp"

/** Test data members. */
AFL_TEST("game.ShipBuildOrder:basics", a)
{
    // Test initial state
    game::ShipBuildOrder testee;
    a.checkEqual("01. getHullIndex",    testee.getHullIndex(), 0);
    a.checkEqual("02. getEngineType",   testee.getEngineType(), 0);
    a.checkEqual("03. getBeamType",     testee.getBeamType(), 0);
    a.checkEqual("04. getNumBeams",     testee.getNumBeams(), 0);
    a.checkEqual("05. getTorpedoType",  testee.getTorpedoType(), 0);
    a.checkEqual("06. getNumLaunchers", testee.getNumLaunchers(), 0);

    // Configure
    testee.setHullIndex(15);
    testee.setEngineType(9);
    testee.setBeamType(3);
    testee.setNumBeams(18);
    testee.setTorpedoType(8);
    testee.setNumLaunchers(4);

    // Verify
    a.checkEqual("11. getHullIndex",    testee.getHullIndex(), 15);
    a.checkEqual("12. getEngineType",   testee.getEngineType(), 9);
    a.checkEqual("13. getBeamType",     testee.getBeamType(), 3);
    a.checkEqual("14. getNumBeams",     testee.getNumBeams(), 18);
    a.checkEqual("15. getTorpedoType",  testee.getTorpedoType(), 8);
    a.checkEqual("16. getNumLaunchers", testee.getNumLaunchers(), 4);
    a.checkEqual("17. toScriptCommand", testee.toScriptCommand("Make", 0), "Make 15, 9, 3, 18, 8, 4");
}

/** Test comparison. */
AFL_TEST("game.ShipBuildOrder:comparison", a)
{
    // Default
    a.checkEqual("01. eq", game::ShipBuildOrder() == game::ShipBuildOrder(), true);
    a.checkEqual("02. ne", game::ShipBuildOrder() != game::ShipBuildOrder(), false);

    // Nondefault
    game::ShipBuildOrder testee;
    testee.setHullIndex(15);
    testee.setEngineType(9);
    testee.setBeamType(3);
    testee.setNumBeams(18);
    testee.setTorpedoType(8);
    testee.setNumLaunchers(4);
    a.checkEqual("11. eq", testee == testee, true);
    a.checkEqual("12. eq", testee == game::ShipBuildOrder(), false);
}

/** Test canonicalize(). */
AFL_TEST("game.ShipBuildOrder:canonicalize", a)
{
    game::ShipBuildOrder testee;
    testee.setHullIndex(15);
    testee.setEngineType(9);
    testee.setBeamType(3);
    testee.setNumBeams(0);
    testee.setTorpedoType(8);
    testee.setNumLaunchers(0);
    testee.canonicalize();

    a.checkEqual("01. getTorpedoType", testee.getTorpedoType(), 0);
    a.checkEqual("02. getBeamType",    testee.getBeamType(), 0);
}

AFL_TEST("game.ShipBuildOrder:describe:torper", a)
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
    testee.setTorpedoType(8);
    testee.setNumLaunchers(1);

    afl::data::StringList_t result;
    afl::string::NullTranslator tx;
    testee.describe(result, sl, tx);

    a.checkEqual("01. size",   result.size(), 4U);
    a.checkEqual("02. hull",   result[0], "ANNIHILATION CLASS BATTLESHIP");
    a.checkEqual("03. engine", result[1], "6 \xC3\x97 Nova Drive 5");
    a.checkEqual("04. beam",   result[2], "4 \xC3\x97 Plasma Bolt");
    a.checkEqual("05. torp",   result[3], "Mark 6 Photon");
    a.checkEqual("06. toScriptCommand", testee.toScriptCommand("Make", &sl), "Make 53, 5, 3, 4, 8, 1   % Annihilation");
}

AFL_TEST("game.ShipBuildOrder:describe:carrier", a)
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
    testee.setTorpedoType(10);
    testee.setNumLaunchers(0);

    afl::data::StringList_t result;
    afl::string::NullTranslator tx;
    testee.describe(result, sl, tx);

    a.checkEqual("01. size",   result.size(), 4U);
    a.checkEqual("02. hull",   result[0], "GORBIE CLASS BATTLECARRIER");
    a.checkEqual("03. engine", result[1], "6 \xC3\x97 Transwarp Drive");
    a.checkEqual("04. beam",   result[2], "Heavy Phaser");
    a.checkEqual("05. bays",   result[3], "10 fighter bays");
}
