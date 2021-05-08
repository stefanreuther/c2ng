/**
  *  \file u/t_game_interface_basetaskpredictor.cpp
  *  \brief Test for game::interface::BaseTaskPredictor
  */

#include "game/interface/basetaskpredictor.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/test/shiplist.hpp"
#include "interpreter/arguments.hpp"

namespace {
    struct TestHarness {
        game::map::Planet planet;
        game::map::Universe univ;
        game::spec::ShipList shipList;
        game::config::HostConfiguration config;

        TestHarness()
            : planet(99), univ(), shipList(), config()
            { }
    };

    const int PLAYER = 4;
    const int HULL_SLOT = 7;

    void prepare(TestHarness& h)
    {
        afl::string::NullTranslator tx;
        afl::sys::Log log;

        // Populate ship list
        game::test::addGorbie(h.shipList);
        game::test::initStandardBeams(h.shipList);
        game::test::initStandardTorpedoes(h.shipList);
        game::test::addTranswarp(h.shipList);
        h.shipList.hullAssignments().add(PLAYER, HULL_SLOT, game::test::GORBIE_HULL_ID);

        // Planet
        game::map::PlanetData pd;
        pd.owner = PLAYER;
        pd.friendlyCode = "xxx";
        pd.colonistClans = 100;
        pd.colonistHappiness = 100;
        pd.money = 1000;
        pd.supplies = 1000;
        pd.minedNeutronium = 1000;
        pd.minedTritanium = 1000;
        pd.minedDuranium = 1000;
        pd.minedMolybdenum = 1000;
        h.planet.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));

        game::map::BaseData bd;
        bd.mission = 0;
        for (int i = 1; i <= 20; ++i) {
            bd.hullStorage.set(i, 0);
            bd.engineStorage.set(i, 0);
            bd.beamStorage.set(i, 0);
            bd.launcherStorage.set(i, 0);
        }
        h.planet.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER));
        h.planet.internalCheck(h.univ.config(), tx, log);
        h.planet.combinedCheck2(h.univ, game::PlayerSet_t(PLAYER), 77);
        h.planet.setPlayability(game::map::Object::Playable);
    }

    void addShip(TestHarness& h, int id)
    {
        game::map::Ship& sh = *h.univ.ships().create(id);

        game::map::ShipData sd;
        sd.hullType = game::test::GORBIE_HULL_ID;
        sd.beamType = 2;
        sd.numBeams = 5;
        sd.numBays = 10;
        sd.engineType = 9;
        sd.owner = PLAYER;
        sd.x = 1000;
        sd.y = 1000;
        sd.neutronium = 100;
        sd.friendlyCode = "abc";
        sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
        sh.internalCheck();
        sh.combinedCheck1(h.univ, game::PlayerSet_t(PLAYER), 77);
        sh.setPlayability(game::map::Object::Playable);
    }
}

/** Test build order prediction.
    A: create planet with build order. Call advanceTurn().
    E: build order cleared, components removed from storage */
void
TestGameInterfaceBaseTaskPredictor::testBuild()
{
    // Prepare: planet with build order
    TestHarness h;
    prepare(h);

    h.planet.setBaseStorage(game::HullTech, HULL_SLOT, 2);
    h.planet.setBaseStorage(game::EngineTech, 9, 20);
    h.planet.setBaseStorage(game::BeamTech, 5, 10);
    h.planet.setBaseStorage(game::BeamTech, 4, 10);

    game::ShipBuildOrder o;
    o.setHullIndex(HULL_SLOT);
    o.setEngineType(9);
    o.setNumBeams(8);
    o.setBeamType(5);
    h.planet.setBaseBuildOrder(o);

    // Action
    game::interface::BaseTaskPredictor testee(h.planet, h.univ, h.shipList, h.config);
    testee.advanceTurn();

    // Verify
    TS_ASSERT_EQUALS(testee.planet().getBaseBuildOrder().getHullIndex(), 0);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 1);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::EngineTech, 9).orElse(-1), 14);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::BeamTech, 5).orElse(-1), 2);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::BeamTech, 4).orElse(-1), 10);
}

/** Test dump order prediction.
    A: create planet with dump order. Call advanceTurn().
    E: storage cleared */
void
TestGameInterfaceBaseTaskPredictor::testRecycle()
{
    // Prepare: planet with "dmp" order
    TestHarness h;
    prepare(h);
    h.planet.setBaseStorage(game::HullTech, HULL_SLOT, 2);
    h.planet.setBaseStorage(game::EngineTech, 9, 20);
    h.planet.setBaseStorage(game::BeamTech, 5, 10);
    h.planet.setBaseStorage(game::BeamTech, 4, 10);
    h.planet.setFriendlyCode(String_t("dmp"));

    // Action
    game::interface::BaseTaskPredictor testee(h.planet, h.univ, h.shipList, h.config);
    testee.advanceTurn();

    // Verify
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 0);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::EngineTech, 9).orElse(-1), 0);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::BeamTech, 5).orElse(-1), 0);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::BeamTech, 4).orElse(-1), 0);
}

/** Test shipyard order prediction.
    A: create planet with shipyard recycle order. Call advanceTurn().
    E: parts added to storage (but not the hull!) */
void
TestGameInterfaceBaseTaskPredictor::testShipyard()
{
    // Prepare: planet with recycle order, ship
    TestHarness h;
    prepare(h);
    addShip(h, 33);
    h.planet.setBaseShipyardOrder(game::RecycleShipyardAction, 33);

    // Action
    game::interface::BaseTaskPredictor testee(h.planet, h.univ, h.shipList, h.config);
    testee.advanceTurn();

    // Verify
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 0);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::EngineTech, 9).orElse(-1), 6);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::BeamTech, 2).orElse(-1), 5);
}

/** Test "BuildShip" command.
    A: create planet. Predict a "BuildShip" command.
    E: command is added to planet */
void
TestGameInterfaceBaseTaskPredictor::testBuildShipCommand()
{
    // Prepare: planet, 'build ship' command
    TestHarness h;
    prepare(h);

    afl::data::Segment seg;
    seg.pushBackInteger(game::test::GORBIE_HULL_ID);
    seg.pushBackInteger(9);
    interpreter::Arguments args(seg, 0, 2);

    // Action
    game::interface::BaseTaskPredictor testee(h.planet, h.univ, h.shipList, h.config);
    bool ok = testee.predictInstruction("BUILDSHIP", args);
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 1);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::EngineTech, 9).orElse(-1), 6);
    TS_ASSERT_EQUALS(testee.planet().getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    TS_ASSERT_EQUALS(testee.planet().getBaseBuildOrder().getEngineType(), 9);
}

/** Test "SetFCode" command.
    A: create planet. Predict a "SetFCode" command.
    E: friendly code is changed */
void
TestGameInterfaceBaseTaskPredictor::testSetFCodeCommand()
{
    // Prepare: planet, 'set fcode' command
    TestHarness h;
    prepare(h);

    afl::data::Segment seg;
    seg.pushBackString("hi!");
    interpreter::Arguments args(seg, 0, 1);

    // Action
    game::interface::BaseTaskPredictor testee(h.planet, h.univ, h.shipList, h.config);
    bool ok = testee.predictInstruction("SETFCODE", args);
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.planet().getFriendlyCode().orElse(""), "hi!");
}

/** Test "SetMission" command.
    A: create planet. Predict a "SetMission" command.
    E: mission is changed */
void
TestGameInterfaceBaseTaskPredictor::testSetMissionCommand()
{
    // Prepare: planet, 'set mission' command
    TestHarness h;
    prepare(h);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    interpreter::Arguments args(seg, 0, 1);

    // Action
    game::interface::BaseTaskPredictor testee(h.planet, h.univ, h.shipList, h.config);
    bool ok = testee.predictInstruction("SETMISSION", args);
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.planet().getBaseMission().orElse(-1), 3);
}

/** Test "FixShip" command.
    A: create planet. Predict a "FixShip" command.
    E: command is added to planet */
void
TestGameInterfaceBaseTaskPredictor::testFixShipCommand()
{
    // Prepare: planet, 'fix ship' command
    TestHarness h;
    prepare(h);
    addShip(h, 123);

    afl::data::Segment seg;
    seg.pushBackInteger(123);
    interpreter::Arguments args(seg, 0, 1);

    // Action
    game::interface::BaseTaskPredictor testee(h.planet, h.univ, h.shipList, h.config);
    bool ok = testee.predictInstruction("FIXSHIP", args);
    TS_ASSERT(ok);

    // Verify
    TS_ASSERT_EQUALS(testee.planet().getBaseShipyardAction().orElse(-1), game::FixShipyardAction);
    TS_ASSERT_EQUALS(testee.planet().getBaseShipyardId().orElse(-1), 123);
}

