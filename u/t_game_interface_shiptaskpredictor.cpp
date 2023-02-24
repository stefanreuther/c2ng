/**
  *  \file u/t_game_interface_shiptaskpredictor.cpp
  *  \brief Test for game::interface::ShipTaskPredictor
  */

#include "game/interface/shiptaskpredictor.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
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
void
TestGameInterfaceShipTaskPredictor::testMovement()
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
    TS_ASSERT_EQUALS(testee.getPosition(0).getX(), 1049);
    TS_ASSERT_EQUALS(testee.getPosition().getX(), 1049);
    TS_ASSERT_EQUALS(testee.getMovementFuel(), 55);
    TS_ASSERT_EQUALS(testee.getRemainingFuel(), 95);
    TS_ASSERT_EQUALS(testee.getNumTurns(), 1);
    TS_ASSERT_EQUALS(testee.getNumFuelTurns(), 1);
    TS_ASSERT_EQUALS(testee.getNumPositions(), 1U);
    TS_ASSERT_EQUALS(testee.getNumFuelPositions(), 1U);
    TS_ASSERT_EQUALS(testee.getWarpFactor(), 7);

    // Advance some more turns; verify
    for (int i = 0; i < 6; ++i) {
        testee.advanceTurn();
    }

    TS_ASSERT_EQUALS(testee.getMovementFuel(), 349);
    TS_ASSERT_EQUALS(testee.getRemainingFuel(), 0);
    TS_ASSERT_EQUALS(testee.getPosition(0).getX(), 1049);
    TS_ASSERT_EQUALS(testee.getPosition(1).getX(), 1098);
    TS_ASSERT_EQUALS(testee.getPosition(2).getX(), 1147);
    TS_ASSERT_EQUALS(testee.getPosition().getX(), 1343);
    TS_ASSERT_EQUALS(testee.getNumTurns(), 7);
    TS_ASSERT_EQUALS(testee.getNumFuelTurns(), 2);
    TS_ASSERT_EQUALS(testee.getNumPositions(), 7U);
    TS_ASSERT_EQUALS(testee.getNumFuelPositions(), 2U);
}

/** Test "MoveTo" command.
    A: create ship. Predict "MoveTo" command.
    E: correct movement predicted */
void
TestGameInterfaceShipTaskPredictor::testMoveToCommand()
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
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.getMovementFuel(), 155);
    TS_ASSERT_EQUALS(testee.getRemainingFuel(), 845);
    TS_ASSERT_EQUALS(testee.getPosition(0).getX(), 1049);
    TS_ASSERT_EQUALS(testee.getPosition(1).getX(), 1080);
    TS_ASSERT_EQUALS(testee.getPosition().getX(), 1080);
    TS_ASSERT_EQUALS(testee.getNumTurns(), 2);
    TS_ASSERT_EQUALS(testee.getNumFuelTurns(), 2);
    TS_ASSERT_EQUALS(testee.getNumPositions(), 2U);
    TS_ASSERT_EQUALS(testee.getNumFuelPositions(), 2U);
    TS_ASSERT_EQUALS(testee.isHyperdriving(), false);
}

/** Test "SetWaypoint" command.
    A: create ship. Predict "SetWaypoint" command.
    E: correct movement predicted */
void
TestGameInterfaceShipTaskPredictor::testSetWaypointCommand()
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
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumPositions(), 0U);
    TS_ASSERT_EQUALS(testee.getNumFuelPositions(), 0U);
    TS_ASSERT_EQUALS(testee.getPosition().getX(), 1000);

    // Move; then verify again
    testee.advanceTurn();
    TS_ASSERT_EQUALS(testee.getNumPositions(), 1U);
    TS_ASSERT_EQUALS(testee.getPosition().getX(), 1049);
}

/** Test "MoveTowards" command.
    A: create ship. Predict "MoveTowards" command.
    E: correct movement predicted */
void
TestGameInterfaceShipTaskPredictor::testMoveTowardsCommand()
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
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.getMovementFuel(), 97);
    TS_ASSERT_EQUALS(testee.getRemainingFuel(), 903);
    TS_ASSERT_EQUALS(testee.getPosition(0).getX(), 1049);
    TS_ASSERT_EQUALS(testee.getPosition().getX(), 1049);
    TS_ASSERT_EQUALS(testee.getNumTurns(), 1);
    TS_ASSERT_EQUALS(testee.getNumFuelTurns(), 1);
    TS_ASSERT_EQUALS(testee.getNumPositions(), 1U);
    TS_ASSERT_EQUALS(testee.getNumFuelPositions(), 1U);
}

/** Test "SetSpeed" command.
    A: create ship. Predict "SetSpeed" command.
    E: warp factor taken over */
void
TestGameInterfaceShipTaskPredictor::testSetSpeedCommand()
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
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumPositions(), 0U);
    TS_ASSERT_EQUALS(testee.getWarpFactor(), 5);
}

/** Test "SetFCode" command.
    A: create ship. Predict "SetFCode" command.
    E: friendly code taken over */
void
TestGameInterfaceShipTaskPredictor::testSetFCodeCommand()
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
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumPositions(), 0U);
    TS_ASSERT_EQUALS(testee.getFriendlyCode(), "ppp");
}

/** Test "SetMission" command.
    A: create ship. Predict "SetMission" command with an Intercept mission (other missions have no external effect).
    E: correct movement predicted */
void
TestGameInterfaceShipTaskPredictor::testSetMissionCommand()
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
    TS_ASSERT(ok);

    testee.advanceTurn();

    // Verify
    TS_ASSERT_EQUALS(testee.getMovementFuel(), 67);
    TS_ASSERT_EQUALS(testee.getRemainingFuel(), 333);
    TS_ASSERT_EQUALS(testee.getPosition(0).getX(), 1172);
    TS_ASSERT_EQUALS(testee.getPosition().getX(), 1172);
    TS_ASSERT_EQUALS(testee.getNumTurns(), 1);
    TS_ASSERT_EQUALS(testee.getNumFuelTurns(), 1);
    TS_ASSERT_EQUALS(testee.getNumPositions(), 1U);
    TS_ASSERT_EQUALS(testee.getNumFuelPositions(), 1U);
}

/** Test "SetFCode" command.
    A: create ship. Predict "SetFCode" command.
    E: friendly code taken over */
void
TestGameInterfaceShipTaskPredictor::testSetFCodeHyperjump()
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
    TS_ASSERT(!testee.isHyperdriving());
    bool ok = testee.predictInstruction("SETFCODE", args);
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumPositions(), 0U);
    TS_ASSERT_EQUALS(testee.getFriendlyCode(), "HYP");
    TS_ASSERT(testee.isHyperdriving());
}

