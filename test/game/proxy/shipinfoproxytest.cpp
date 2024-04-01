/**
  *  \file test/game/proxy/shipinfoproxytest.cpp
  *  \brief Test for game::proxy::ShipInfoProxy
  */

#include "game/proxy/shipinfoproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "game/unitscoredefinitionlist.hpp"

using afl::base::Ptr;
using game::Game;
using game::HostVersion;
using game::Root;
using game::UnitScoreDefinitionList;
using game::config::HostConfiguration;
using game::map::Point;
using game::map::Ship;
using game::proxy::ShipInfoProxy;
using game::spec::Hull;
using game::spec::ShipList;
using game::test::SessionThread;

namespace {
    Root& addRoot(SessionThread& t)
    {
        Ptr<Root> r = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,1,0))).asPtr();
        t.session().setRoot(r);
        return *r;
    }

    Game& addGame(SessionThread& t)
    {
        Ptr<Game> g = new Game();
        t.session().setGame(g);
        return *g;
    }

    ShipList& addShipList(SessionThread& t)
    {
        Ptr<ShipList> sl = new ShipList();
        t.session().setShipList(sl);
        return *sl;
    }

    void setupCargoTest(SessionThread& t, int shipId, int hullNr, Ship::Playability pl)
    {
        // Root
        addRoot(t);

        // Shiplist
        ShipList& sl = addShipList(t);
        Hull& h = *sl.hulls().create(hullNr);
        h.setMass(300);
        h.setMaxCargo(2000);
        h.setMaxFuel(150);

        // Ship
        Game& g = addGame(t);
        Ship& sh = *g.currentTurn().universe().ships().create(shipId);
        sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
        sh.setHull(hullNr);                                                  // enables mass ranges
        sh.setCargo(game::Element::Tritanium, 20);                           // enables cargo
        sh.internalCheck(game::PlayerSet_t(4), 15);
        sh.setPlayability(pl);
    }
}

/*
 *  getCargo
 */

// Empty session
AFL_TEST("game.proxy.ShipInfoProxy:getCargo:empty", a)
{
    SessionThread t;
    game::test::WaitIndicator ind;
    ShipInfoProxy testee(t.gameSender());

    game::map::ShipCargoInfos_t out;
    ShipInfoProxy::CargoStatus st = testee.getCargo(ind, 100, ShipInfoProxy::GetLastKnownCargo | ShipInfoProxy::GetMassRanges, out);
    a.checkEqual("01", st, ShipInfoProxy::NoCargo);
    a.checkEqual("02", out.size(), 0U);
}

// Scanned ship
AFL_TEST("game.proxy.ShipInfoProxy:getCargo:scanned", a)
{
    const int SHIP_ID = 10;

    SessionThread t;
    game::test::WaitIndicator ind;
    setupCargoTest(t, SHIP_ID, 22, Ship::NotPlayable);

    ShipInfoProxy testee(t.gameSender());

    game::map::ShipCargoInfos_t cargoResult, massResult, combinedResult;
    a.checkEqual("01. cargo",    testee.getCargo(ind, SHIP_ID, ShipInfoProxy::GetLastKnownCargo,                                cargoResult),    ShipInfoProxy::HistoryCargo);
    a.checkEqual("02. mass",     testee.getCargo(ind, SHIP_ID,                                    ShipInfoProxy::GetMassRanges, massResult),     ShipInfoProxy::HistoryCargo);
    a.checkEqual("03. combined", testee.getCargo(ind, SHIP_ID, ShipInfoProxy::GetLastKnownCargo | ShipInfoProxy::GetMassRanges, combinedResult), ShipInfoProxy::HistoryCargo);

    a.checkDifferent("11. cargo",    cargoResult.size(),    0U);
    a.checkDifferent("12. mass",     massResult.size(),     0U);
    a.checkDifferent("13. combined", combinedResult.size(), 0U);

    a.checkGreaterEqual("21. total", cargoResult.size() + massResult.size(), combinedResult.size());
}

// Played ship
AFL_TEST("game.proxy.ShipInfoProxy:getCargo:played", a)
{
    const int SHIP_ID = 20;

    SessionThread t;
    game::test::WaitIndicator ind;
    setupCargoTest(t, SHIP_ID, 33, Ship::Playable);

    ShipInfoProxy testee(t.gameSender());

    game::map::ShipCargoInfos_t cargoResult, massResult, combinedResult;
    a.checkEqual("01. cargo",    testee.getCargo(ind, SHIP_ID, ShipInfoProxy::GetLastKnownCargo,                                cargoResult),    ShipInfoProxy::CurrentShip);
    a.checkEqual("02. mass",     testee.getCargo(ind, SHIP_ID,                                    ShipInfoProxy::GetMassRanges, massResult),     ShipInfoProxy::CurrentShip);
    a.checkEqual("03. combined", testee.getCargo(ind, SHIP_ID, ShipInfoProxy::GetLastKnownCargo | ShipInfoProxy::GetMassRanges, combinedResult), ShipInfoProxy::CurrentShip);

    a.checkEqual("11. cargo",    cargoResult.size(),    0U);
    a.checkEqual("12. mass",     massResult.size(),     0U);
    a.checkEqual("13. combined", combinedResult.size(), 0U);
}

/*
 *  getExperienceInfo
 */

// Empty session
AFL_TEST("game.proxy.ShipInfoProxy:getExperienceInfo:empty", a)
{
    SessionThread t;
    game::test::WaitIndicator ind;
    ShipInfoProxy testee(t.gameSender());

    game::map::ShipExperienceInfo exp = testee.getExperienceInfo(ind, 100);
    a.check("01. level",       !exp.level.isValid());
    a.check("02. points",      !exp.points.isValid());
    a.check("03. pointGrowth", !exp.pointGrowth.isValid());
}

// Normal ship
AFL_TEST("game.proxy.ShipInfoProxy:getExperienceInfo:ship", a)
{
    const int SHIP_ID = 42;
    const int HULL_NR = 2;

    SessionThread t;
    game::test::WaitIndicator ind;

    // Root
    Root& r = addRoot(t);
    r.hostConfiguration()[HostConfiguration::NumExperienceLevels].set(4);

    // Shiplist
    ShipList& sl = addShipList(t);
    Hull& h = *sl.hulls().create(HULL_NR);
    h.setMaxCrew(100);

    // Game
    Game& g = addGame(t);
    UnitScoreDefinitionList::Definition ldef;
    ldef.name = "Level";
    ldef.id = game::ScoreId_ExpLevel;
    ldef.limit = 4;
    UnitScoreDefinitionList::Index_t lindex = g.shipScores().add(ldef);

    // Ship
    Ship& sh = *g.currentTurn().universe().ships().create(SHIP_ID);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.setHull(HULL_NR);
    sh.internalCheck(game::PlayerSet_t(4), 15);
    sh.setPlayability(Ship::Playable);
    sh.unitScores().set(lindex, 3, 15);

    // Test
    ShipInfoProxy testee(t.gameSender());
    game::map::ShipExperienceInfo exp = testee.getExperienceInfo(ind, SHIP_ID);
    a.checkEqual("01. level", exp.level.orElse(-1), 3);
}

