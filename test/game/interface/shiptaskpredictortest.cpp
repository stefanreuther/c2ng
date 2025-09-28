/**
  *  \file test/game/interface/shiptaskpredictortest.cpp
  *  \brief Test for game::interface::ShipTaskPredictor
  */

#include "game/interface/shiptaskpredictor.hpp"

#include "afl/data/segment.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/hostversion.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "interpreter/arguments.hpp"

namespace {
    struct TestHarness {
        game::map::Universe univ;
        game::map::Configuration mapConfig;
        game::UnitScoreDefinitionList scoreDefinitions;
        game::spec::ShipList shipList;
        game::config::HostConfiguration config;
        game::HostVersion hostVersion;
        game::test::RegistrationKey key;

        TestHarness()
            : univ(), mapConfig(), scoreDefinitions(), shipList(), config(), hostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)),
              key(game::RegistrationKey::Registered, 10)
            { }
    };

    const int PLAYER = 4;
    const int HULL_SLOT = 7;
    const int INTERCEPT = 44;      /* deliberately NOT the standard intercept mission to verify that we check attributes, not numbers */

    void prepare(TestHarness& h)
    {
        // Populate ship list
        game::test::addGorbie(h.shipList);
        game::test::initStandardBeams(h.shipList);
        game::test::initStandardTorpedoes(h.shipList);
        game::test::addTranswarp(h.shipList);
        h.shipList.hullAssignments().add(PLAYER, HULL_SLOT, game::test::GORBIE_HULL_ID);

        // Add intercept mission
        h.shipList.missions().addMission(game::spec::Mission(INTERCEPT, "!is*,Intercept"));
    }

    game::map::Ship& addShip(TestHarness& h, int id)
    {
        game::map::Ship& sh = *h.univ.ships().create(id);

        game::map::ShipData sd;
        sd.hullType = game::test::GORBIE_HULL_ID;
        sd.beamType = 2;
        sd.numBeams = 5;
        sd.numBays = 10;
        sd.numLaunchers = 0;
        sd.torpedoType = 0;
        sd.engineType = 9;
        sd.owner = PLAYER;
        sd.x = 1000;
        sd.y = 1000;
        sd.neutronium = 100;
        sd.friendlyCode = "abc";
        sd.tritanium = 0;
        sd.duranium = 0;
        sd.molybdenum = 0;
        sd.supplies = 0;
        sd.ammo = 0;
        sd.colonists = 0;
        sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
        sh.internalCheck(game::PlayerSet_t(PLAYER), 77);
        sh.setPlayability(game::map::Object::Playable);

        return sh;
    }
}

/** Test movement.
    A: create a ship with movement order. Call advanceTurn() repeatedly.
    E: correct positions and other results produced */
AFL_TEST("game.interface.ShipTaskPredictor:movement", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWaypoint(game::map::Point(2000, 1000));
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 150);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);

    // Advance one turn; verify
    testee.advanceTurn();
    a.checkEqual("01. position 0 X",        testee.getPosition(0).getX(), 1049);
    a.checkEqual("02. position X",          testee.getPosition().getX(), 1049);
    a.checkEqual("03. getMovementFuel",     testee.getMovementFuel(), 55);
    a.checkEqual("04. getRemainingFuel",    testee.getRemainingFuel(), 95);
    a.checkEqual("05. getNumTurns",         testee.getNumTurns(), 1);
    a.checkEqual("06. getNumFuelTurns",     testee.getNumFuelTurns(), 1);
    a.checkEqual("07. getNumPositions",     testee.getNumPositions(), 1U);
    a.checkEqual("08. getNumFuelPositions", testee.getNumFuelPositions(), 1U);
    a.checkEqual("09. getWarpFactor",       testee.getWarpFactor(), 7);

    // Advance some more turns; verify
    for (int i = 0; i < 6; ++i) {
        testee.advanceTurn();
    }

    a.checkEqual("11. getMovementFuel",     testee.getMovementFuel(), 349);
    a.checkEqual("12. getRemainingFuel",    testee.getRemainingFuel(), 0);
    a.checkEqual("13. position 0 X",        testee.getPosition(0).getX(), 1049);
    a.checkEqual("14. position 1 X",        testee.getPosition(1).getX(), 1098);
    a.checkEqual("15. position 2 X",        testee.getPosition(2).getX(), 1147);
    a.checkEqual("16. position X",          testee.getPosition().getX(), 1343);
    a.checkEqual("17. getNumTurns",         testee.getNumTurns(), 7);
    a.checkEqual("18. getNumFuelTurns",     testee.getNumFuelTurns(), 2);
    a.checkEqual("19. getNumPositions",     testee.getNumPositions(), 7U);
    a.checkEqual("20. getNumFuelPositions", testee.getNumFuelPositions(), 2U);
}

/** Test "MoveTo" command.
    A: create ship. Predict "MoveTo" command.
    E: correct movement predicted */
AFL_TEST("game.interface.ShipTaskPredictor:command:MoveTo", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 1000);

    afl::data::Segment seg;
    seg.pushBackInteger(1080);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 2);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);
    bool ok = testee.predictInstruction("MOVETO", args);
    a.check("01. predictInstruction", ok);

    // Verify
    a.checkEqual("11. getMovementFuel",     testee.getMovementFuel(), 155);
    a.checkEqual("12. getRemainingFuel",    testee.getRemainingFuel(), 845);
    a.checkEqual("13. position 0 X",        testee.getPosition(0).getX(), 1049);
    a.checkEqual("14. position 1 X",        testee.getPosition(1).getX(), 1080);
    a.checkEqual("15. position X",          testee.getPosition().getX(), 1080);
    a.checkEqual("16. getNumTurns",         testee.getNumTurns(), 2);
    a.checkEqual("17. getNumFuelTurns",     testee.getNumFuelTurns(), 2);
    a.checkEqual("18. getNumPositions",     testee.getNumPositions(), 2U);
    a.checkEqual("19. getNumFuelPositions", testee.getNumFuelPositions(), 2U);
    a.checkEqual("20. isHyperdriving",      testee.isHyperdriving(), false);
}

/** Test "MoveTo" command, SimpleMovement version.
    A: create ship. Predict "MoveTo" command in SimpleMovement.
    E: correct movement predicted */
AFL_TEST("game.interface.ShipTaskPredictor:command:MoveTo:simple", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 1000);

    afl::data::Segment seg;
    seg.pushBackInteger(1080);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 2);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);
    testee.setMovementMode(game::interface::ShipTaskPredictor::SimpleMovement);
    bool ok = testee.predictInstruction("MOVETO", args);
    a.check("01. predictInstruction", ok);

    // Verify
    a.checkEqual("11. getNumPositions",     testee.getNumPositions(), 1U);
    a.checkEqual("12. position 1 X",        testee.getPosition(0).getX(), 1080);
    a.checkEqual("13. position X",          testee.getPosition().getX(), 1080);
}

/** Test "MoveTo" command, SimpleMovement version with gravity.
    A: create ship. Add planet close to ship waypoint. Predict "MoveTo" command in SimpleMovement.
    E: correct movement predicted */
AFL_TEST("game.interface.ShipTaskPredictor:command:MoveTo:gravity", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 1000);

    // Create planet
    {
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        game::map::Planet* pl = h.univ.planets().create(42);
        pl->setPosition(game::map::Point(1082, 1000));
        pl->internalCheck(h.mapConfig, game::PlayerSet_t(1), 10, tx, log);
    }

    // Command
    afl::data::Segment seg;
    seg.pushBackInteger(1080);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 2);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);
    testee.setMovementMode(game::interface::ShipTaskPredictor::SimpleMovement);
    bool ok = testee.predictInstruction("MOVETO", args);
    a.check("01. predictInstruction", ok);

    // Verify
    a.checkEqual("11. getNumPositions",     testee.getNumPositions(), 1U);
    a.checkEqual("12. position 1 X",        testee.getPosition(0).getX(), 1082);
    a.checkEqual("13. position X",          testee.getPosition().getX(), 1082);
}

/** Test "SetWaypoint" command.
    A: create ship. Predict "SetWaypoint" command.
    E: correct movement predicted */
AFL_TEST("game.interface.ShipTaskPredictor:command:SetWaypoint", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 1000);

    afl::data::Segment seg;
    seg.pushBackInteger(1080);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 2);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);
    bool ok = testee.predictInstruction("SETWAYPOINT", args);
    a.check("01. predictInstruction", ok);

    // Verify
    a.checkEqual("11. getNumPositions",     testee.getNumPositions(), 0U);
    a.checkEqual("12. getNumFuelPositions", testee.getNumFuelPositions(), 0U);
    a.checkEqual("13. position X",          testee.getPosition().getX(), 1000);

    // Move; then verify again
    testee.advanceTurn();
    a.checkEqual("21. getNumPositions", testee.getNumPositions(), 1U);
    a.checkEqual("22. position X",      testee.getPosition().getX(), 1049);
}

/** Test "MoveTowards" command.
    A: create ship. Predict "MoveTowards" command.
    E: correct movement predicted */
AFL_TEST("game.interface.ShipTaskPredictor:command:MoveTowards", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 1000);

    afl::data::Segment seg;
    seg.pushBackInteger(1080);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 2);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);
    bool ok = testee.predictInstruction("MOVETOWARDS", args);
    a.check("01. predictInstruction", ok);

    // Verify
    a.checkEqual("11. getMovementFuel",     testee.getMovementFuel(), 97);
    a.checkEqual("12. getRemainingFuel",    testee.getRemainingFuel(), 903);
    a.checkEqual("13. position 0 X",        testee.getPosition(0).getX(), 1049);
    a.checkEqual("14. position X",          testee.getPosition().getX(), 1049);
    a.checkEqual("15. getNumTurns",         testee.getNumTurns(), 1);
    a.checkEqual("16. getNumFuelTurns",     testee.getNumFuelTurns(), 1);
    a.checkEqual("17. getNumPositions",     testee.getNumPositions(), 1U);
    a.checkEqual("18. getNumFuelPositions", testee.getNumFuelPositions(), 1U);
}

/** Test "SetSpeed" command.
    A: create ship. Predict "SetSpeed" command.
    E: warp factor taken over */
AFL_TEST("game.interface.ShipTaskPredictor:command:SetSpeed", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 1000);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 1);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);
    bool ok = testee.predictInstruction("SETSPEED", args);
    a.check("01. predictInstruction", ok);

    // Verify
    a.checkEqual("11. getNumPositions", testee.getNumPositions(), 0U);
    a.checkEqual("12. getWarpFactor",   testee.getWarpFactor(), 5);
}

/** Test "SetFCode" command.
    A: create ship. Predict "SetFCode" command.
    E: friendly code taken over */
AFL_TEST("game.interface.ShipTaskPredictor:command:SetFCode", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 1000);

    afl::data::Segment seg;
    seg.pushBackString("ppp");
    interpreter::Arguments args(seg, 0, 1);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);
    bool ok = testee.predictInstruction("SETFCODE", args);
    a.check("01. predictInstruction", ok);

    // Verify
    a.checkEqual("11. getNumPositions", testee.getNumPositions(), 0U);
    a.checkEqual("12. getFriendlyCode", testee.getFriendlyCode(), "ppp");
}

/** Test "SetMission" command.
    A: create ship. Predict "SetMission" command with an Intercept mission (other missions have no external effect).
    E: correct movement predicted */
AFL_TEST("game.interface.ShipTaskPredictor:command:SetMission", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 400);

    /*game::map::Ship& other =*/ addShip(h, 555);
    sh.setPosition(game::map::Point(1200, 1300));

    afl::data::Segment seg;
    seg.pushBackInteger(INTERCEPT);
    seg.pushBackInteger(555);
    seg.pushBackInteger(0);
    interpreter::Arguments args(seg, 0, 3);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);
    bool ok = testee.predictInstruction("SETMISSION", args);
    a.check("01. predictInstruction", ok);

    testee.advanceTurn();

    // Verify
    a.checkEqual("11. getMovementFuel",     testee.getMovementFuel(), 67);
    a.checkEqual("12. getRemainingFuel",    testee.getRemainingFuel(), 333);
    a.checkEqual("13. position 0 X",        testee.getPosition(0).getX(), 1172);
    a.checkEqual("14. position X",          testee.getPosition().getX(), 1172);
    a.checkEqual("15. getNumTurns",         testee.getNumTurns(), 1);
    a.checkEqual("16. getNumFuelTurns",     testee.getNumFuelTurns(), 1);
    a.checkEqual("17. getNumPositions",     testee.getNumPositions(), 1U);
    a.checkEqual("18. getNumFuelPositions", testee.getNumFuelPositions(), 1U);
}

/** Test "SetFCode" command.
    A: create ship. Predict "SetFCode" command.
    E: friendly code taken over */
AFL_TEST("game.interface.ShipTaskPredictor:command:SetFCode:hyperjump", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    game::map::Ship& sh = addShip(h, 99);
    sh.setWarpFactor(7);
    sh.setCargo(game::Element::Neutronium, 1000);
    sh.addShipSpecialFunction(h.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Hyperdrive));

    afl::data::Segment seg;
    seg.pushBackString("HYP");
    interpreter::Arguments args(seg, 0, 1);

    // Object under test
    game::interface::ShipTaskPredictor testee(h.univ, 99, h.scoreDefinitions, h.shipList, h.mapConfig, h.config, h.hostVersion, h.key);
    a.check("01. isHyperdriving", !testee.isHyperdriving());
    bool ok = testee.predictInstruction("SETFCODE", args);
    a.check("02. predictInstruction", ok);

    // Verify
    a.checkEqual("11. getNumPositions", testee.getNumPositions(), 0U);
    a.checkEqual("12. getFriendlyCode", testee.getFriendlyCode(), "HYP");
    a.check("13. isHyperdriving", testee.isHyperdriving());
}
