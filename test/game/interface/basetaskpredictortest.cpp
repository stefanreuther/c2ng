/**
  *  \file test/game/interface/basetaskpredictortest.cpp
  *  \brief Test for game::interface::BaseTaskPredictor
  */

#include "game/interface/basetaskpredictor.hpp"

#include "afl/data/segment.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
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
        h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(PLAYER), 77, tx, log);
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
        sh.internalCheck(game::PlayerSet_t(PLAYER), 77);
        sh.setPlayability(game::map::Object::Playable);
    }
}

/** Test build order prediction.
    A: create planet with build order. Call advanceTurn().
    E: build order cleared, components removed from storage */
AFL_TEST("game.interface.BaseTaskPredictor:build", a)
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
    a.checkEqual("getBaseBuildOrder", testee.planet().getBaseBuildOrder().getHullIndex(), 0);
    a.checkEqual("hull storage",      testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 1);
    a.checkEqual("engine storage",    testee.planet().getBaseStorage(game::EngineTech, 9).orElse(-1), 14);
    a.checkEqual("beam storage 5",    testee.planet().getBaseStorage(game::BeamTech, 5).orElse(-1), 2);
    a.checkEqual("beam storage 4",    testee.planet().getBaseStorage(game::BeamTech, 4).orElse(-1), 10);
}

/** Test dump order prediction.
    A: create planet with dump order. Call advanceTurn().
    E: storage cleared */
AFL_TEST("game.interface.BaseTaskPredictor:dmp", a)
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
    a.checkEqual("hull storage",   testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 0);
    a.checkEqual("engine storage", testee.planet().getBaseStorage(game::EngineTech, 9).orElse(-1), 0);
    a.checkEqual("beam storage 5", testee.planet().getBaseStorage(game::BeamTech, 5).orElse(-1), 0);
    a.checkEqual("beam storage 4", testee.planet().getBaseStorage(game::BeamTech, 4).orElse(-1), 0);
}

/** Test shipyard order prediction.
    A: create planet with shipyard recycle order. Call advanceTurn().
    E: parts added to storage (but not the hull!) */
AFL_TEST("game.interface.BaseTaskPredictor:shipyard", a)
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
    a.checkEqual("hull storage",   testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 0);
    a.checkEqual("engine storage", testee.planet().getBaseStorage(game::EngineTech, 9).orElse(-1), 6);
    a.checkEqual("beam storage",   testee.planet().getBaseStorage(game::BeamTech, 2).orElse(-1), 5);
}

/** Test "BuildShip" command.
    A: create planet. Predict a "BuildShip" command.
    E: command is added to planet */
AFL_TEST("game.interface.BaseTaskPredictor:predictInstruction:BuildShip", a)
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
    a.check("predictInstruction ok", ok);

    // Verify
    a.checkEqual("hull storage",   testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 1);
    a.checkEqual("engine storage", testee.planet().getBaseStorage(game::EngineTech, 9).orElse(-1), 6);
    a.checkEqual("getHullIndex",   testee.planet().getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    a.checkEqual("getEngineType",  testee.planet().getBaseBuildOrder().getEngineType(), 9);
}

/** Test "SetFCode" command.
    A: create planet. Predict a "SetFCode" command.
    E: friendly code is changed */
AFL_TEST("game.interface.BaseTaskPredictor:predictInstruction:SetFCode", a)
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
    a.check("predictInstruction ok", ok);

    // Verify
    a.checkEqual("getFriendlyCode", testee.planet().getFriendlyCode().orElse(""), "hi!");
}

/** Test "SetMission" command.
    A: create planet. Predict a "SetMission" command.
    E: mission is changed */
AFL_TEST("game.interface.BaseTaskPredictor:predictInstruction:SetMission", a)
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
    a.check("predictInstruction ok", ok);

    // Verify
    a.checkEqual("getBaseMission", testee.planet().getBaseMission().orElse(-1), 3);
}

/** Test "FixShip" command.
    A: create planet. Predict a "FixShip" command.
    E: command is added to planet */
AFL_TEST("game.interface.BaseTaskPredictor:predictInstruction:FixShip", a)
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
    a.check("predictInstruction ok", ok);

    // Verify
    a.checkEqual("getBaseShipyardAction", testee.planet().getBaseShipyardAction().orElse(-1), game::FixShipyardAction);
    a.checkEqual("getBaseShipyardId", testee.planet().getBaseShipyardId().orElse(-1), 123);
}
