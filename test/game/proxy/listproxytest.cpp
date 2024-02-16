/**
  *  \file test/game/proxy/listproxytest.cpp
  *  \brief Test for game::proxy::ListProxy
  */

#include "game/proxy/listproxy.hpp"

#include "afl/base/ptr.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/shipdata.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using afl::base::Ptr;
using game::Element;
using game::Game;
using game::HostVersion;
using game::Reference;
using game::Root;
using game::config::HostConfiguration;
using game::map::Object;
using game::map::Planet;
using game::map::Point;
using game::map::Ship;
using game::map::ShipData;
using game::ref::List;
using game::spec::Cost;
using game::spec::CostSummary;
using game::spec::Engine;
using game::spec::Hull;
using game::spec::ShipList;
using game::test::SessionThread;

namespace {
    const int HULL_ID = 17;
    const int ENGINE_ID = 4;

    void addShipList(SessionThread& t)
    {
        Ptr<ShipList> sl = new ShipList();
        Hull& h = *sl->hulls().create(HULL_ID);
        h.setNumEngines(2);
        h.setMaxCargo(100);
        h.setMaxFuel(100);
        h.setMaxCrew(100);

        Engine& e = *sl->engines().create(ENGINE_ID);
        e.setTechLevel(5);

        t.session().setShipList(sl);
    }

    Root& addRoot(SessionThread& t)
    {
        if (t.session().getRoot().get() == 0) {
            t.session().setRoot(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0))).asPtr());
        }
        return *t.session().getRoot();
    }

    Game& addGame(SessionThread& t)
    {
        if (t.session().getGame().get() == 0) {
            t.session().setGame(new Game());
        }
        return *t.session().getGame();
    }

    Ship& addShip(SessionThread& t, int id, int owner, int x, int y, Object::Playability playability)
    {
        Ship& sh = *addGame(t).currentTurn().universe().ships().create(id);
        ShipData data;
        data.owner                     = owner;
        data.friendlyCode              = "hi";
        data.x                         = x;
        data.y                         = y;
        data.waypointDX                = 0;
        data.waypointDY                = 0;
        data.engineType                = ENGINE_ID;
        data.hullType                  = HULL_ID;
        data.beamType                  = 0;
        data.torpedoType               = 0;
        data.mission                   = 0;
        data.missionTowParameter       = 0;
        data.missionInterceptParameter = 0;
        data.warpFactor                = 9;

        sh.addCurrentShipData(data, game::PlayerSet_t(owner));
        sh.internalCheck(game::PlayerSet_t(owner), 15);
        sh.setPlayability(playability);
        return sh;
    }

    Planet& addPlanet(SessionThread& t, int id, int x, int y, int owner, String_t name, Object::Playability playability)
    {
        afl::string::NullTranslator tx;
        afl::sys::Log log;

        Game& g = addGame(t);
        Planet& pl = *g.currentTurn().universe().planets().create(id);
        pl.setName(name);
        pl.setPosition(Point(x, y));
        pl.setOwner(owner);
        pl.internalCheck(g.mapConfiguration(), game::PlayerSet_t(owner), 77, tx, log);
        pl.setPlayability(playability);
        return pl;
    }

    // Commonly-used parameters
    const int X = 1500;
    const int Y = 1600;
    const int OWNER = 3;
}


/* Test behaviour on empty session. */
AFL_TEST("game.proxy.ListProxy:empty", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    testee.buildCurrent(ind, Point(1000, 2000), List::Options_t(), 0);
    a.check("01. list",              testee.getList().size() == 0);
    a.check("02. isCurrent",         testee.isCurrent());
    a.check("03. isUniquePlayable", !testee.isUniquePlayable());
    a.check("04. hasRemoteControl", !testee.hasRemoteControl());
    a.check("05. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("06. hasHidingPlanet",  !testee.hasHidingPlanet());

    a.check("11. getCargoSummary", testee.getCargoSummary(ind).getNumItems() == 0);

    testee.buildNext(ind, Point(1000, 2000), 0, List::Options_t());
    a.check("21. list",              testee.getList().size() == 0);
    a.check("22. isCurrent",        !testee.isCurrent());
    a.check("23. isUniquePlayable", !testee.isUniquePlayable());
    a.check("24. hasRemoteControl", !testee.hasRemoteControl());
    a.check("25. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("26. hasHidingPlanet",  !testee.hasHidingPlanet());

    a.check("31. getCargoSummary", testee.getCargoSummary(ind).getNumItems() == 0);
}

/*
 *  buildCurrent
 */

/* Normal behaviour, current ships. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:normal", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    // Those shall be listed:
    addShip(t, 20, OWNER, X, Y, Object::Playable);
    addShip(t, 50, OWNER, X, Y, Object::Playable);
    addShip(t, 80, OWNER, X, Y, Object::Playable);

    // Those shall not be listed:
    addShip(t, 41, OWNER, X+1, Y, Object::Playable);
    addShip(t, 42, OWNER, X, Y+1, Object::Playable);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 0);

    a.checkEqual("01. list", testee.getList().size(), 3U);
    a.checkEqual("02. list", testee.getList()[0], Reference(Reference::Ship, 20));
    a.checkEqual("03. list", testee.getList()[1], Reference(Reference::Ship, 50));
    a.checkEqual("04. list", testee.getList()[2], Reference(Reference::Ship, 80));

    a.check("11. isCurrent",         testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* Normal behaviour, unique ship. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:unique", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    // Those shall be listed:
    addShip(t, 20, OWNER, X, Y, Object::Playable);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 0);

    a.checkEqual("01. list", testee.getList().size(), 1U);
    a.checkEqual("02. list", testee.getList()[0], Reference(Reference::Ship, 20));

    a.check("11. isCurrent",         testee.isCurrent());
    a.check("12. isUniquePlayable",  testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* Normal behaviour, unique ship, not playable. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:unique:not-playable", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    // Those shall be listed:
    addShip(t, 20, OWNER, X, Y, Object::NotPlayable);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(List::IncludeForeignShips), 0);

    a.checkEqual("01. list", testee.getList().size(), 1U);
    a.checkEqual("02. list", testee.getList()[0], Reference(Reference::Ship, 20));

    a.check("11. isCurrent",         testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* Normal behaviour, current ships, with exclusion. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:exclude", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    addShip(t, 20, OWNER, X, Y, Object::Playable);
    addShip(t, 50, OWNER, X, Y, Object::Playable);
    addShip(t, 80, OWNER, X, Y, Object::Playable);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 50);

    a.checkEqual("01. list", testee.getList().size(), 2U);
    a.checkEqual("02. list", testee.getList()[0], Reference(Reference::Ship, 20));
    a.checkEqual("03. list", testee.getList()[1], Reference(Reference::Ship, 80));

    a.check("11. isCurrent",         testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",   testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* Normal behaviour, current ships, with exclusion; excluded ship does not exist. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:exclude:nonexistant", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    addShip(t, 20, OWNER, X, Y, Object::Playable);
    addShip(t, 50, OWNER, X, Y, Object::Playable);
    addShip(t, 80, OWNER, X, Y, Object::Playable);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 51);

    a.checkEqual("01. list", testee.getList().size(), 3U);
    a.checkEqual("02. list", testee.getList()[0], Reference(Reference::Ship, 20));
    a.checkEqual("03. list", testee.getList()[1], Reference(Reference::Ship, 50));
    a.checkEqual("04. list", testee.getList()[2], Reference(Reference::Ship, 80));

    a.check("11. isCurrent",         testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* Normal behaviour, current ships, with exclusion, excluded ship at wrong position. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:exclude:wrong-position", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    addShip(t, 20, OWNER, X, Y, Object::Playable);
    addShip(t, 50, OWNER, X, Y, Object::Playable);
    addShip(t, 51, OWNER, X+1, Y, Object::Playable);
    addShip(t, 80, OWNER, X, Y, Object::Playable);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 51);

    a.checkEqual("01. list", testee.getList().size(), 3U);
    a.checkEqual("02. list", testee.getList()[0], Reference(Reference::Ship, 20));
    a.checkEqual("03. list", testee.getList()[1], Reference(Reference::Ship, 50));
    a.checkEqual("04. list", testee.getList()[2], Reference(Reference::Ship, 80));

    a.check("11. isCurrent",         testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* No ships found. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:none", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    // No ships

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 0);

    a.checkEqual("01. list", testee.getList().size(), 0U);

    a.check("11. isCurrent",         testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* No ships found, but foreign planet present. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:none:foreign-planet", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    addPlanet(t, 10, X, Y, 1, "Melmac", Object::NotPlayable);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 0);

    a.checkEqual("01. list", testee.getList().size(), 0U);

    a.check("11. isCurrent",         testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",   testee.hasHidingPlanet());
    a.checkEqual("15. getHidingPlanetName", testee.getHidingPlanetName(), "Melmac");
}

/* No ships found, but played planet present. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:none:own-planet", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    addPlanet(t, 10, X, Y, 1, "Melmac", Object::Playable);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 0);

    a.checkEqual("01. list", testee.getList().size(), 0U);

    a.check("11. isCurrent",         testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* Forwarding of CPEnableRemote=No */
AFL_TEST("game.proxy.ListProxy:buildCurrent:remote:off", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t).hostConfiguration()[HostConfiguration::CPEnableRemote].set(0);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 0);

    a.check("", !testee.hasRemoteControl());
}

/* Forwarding of CPEnableRemote=Yes */
AFL_TEST("game.proxy.ListProxy:buildCurrent:remote:on", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t).hostConfiguration()[HostConfiguration::CPEnableRemote].set(1);

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 0);

    a.check("", testee.hasRemoteControl());
}

/* Cargo summary. */
AFL_TEST("game.proxy.ListProxy:buildCurrent:getCargoSummary", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    Ship& sh = addShip(t, 20, OWNER, X, Y, Object::Playable);
    sh.setCargo(Element::Tritanium, 10);
    sh.setCargo(Element::Duranium, 20);
    sh.setCargo(Element::Molybdenum, 30);
    sh.setName("Orville");

    Ship& sh2 = addShip(t, 30, OWNER, X, Y, Object::Playable);
    sh2.setName("Serenity");

    testee.buildCurrent(ind, Point(X, Y), List::Options_t(), 0);
    CostSummary sum = testee.getCargoSummary(ind);

    a.checkEqual("01. size", sum.getNumItems(), 2U);
    a.checkEqual("02. id",   sum.get(0)->id, 20);
    a.checkEqual("03. Tri",  sum.get(0)->cost.get(Cost::Tritanium), 10);
    a.checkEqual("04. Dur",  sum.get(0)->cost.get(Cost::Duranium), 20);
    a.checkEqual("05. Mol",  sum.get(0)->cost.get(Cost::Molybdenum), 30);
    a.checkEqual("06. id",   sum.get(0)->name, "Ship #20: Orville");
    a.checkEqual("07. id",   sum.get(1)->name, "Ship #30: Serenity");
}


/*
 *  buildNext
 */

/* Normal behaviour, next-turn ships. */
AFL_TEST("game.proxy.ListProxy:buildNext:normal", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    // Those shall be listed:
    addShip(t, 20, OWNER, X+10, Y, Object::Playable).setWaypoint(Point(X, Y));
    addShip(t, 50, OWNER, X, Y-10, Object::Playable).setWaypoint(Point(X, Y));
    addShip(t, 80, OWNER, X-10, Y, Object::Playable).setWaypoint(Point(X, Y));

    // Those shall not be listed:
    addShip(t, 41, OWNER, X+1, Y, Object::Playable);
    addShip(t, 42, OWNER, X, Y+1, Object::Playable);

    testee.buildNext(ind, Point(X, Y), 0, List::Options_t());

    a.checkEqual("01. list", testee.getList().size(), 3U);
    a.checkEqual("02. list", testee.getList()[0], Reference(Reference::Ship, 20));
    a.checkEqual("03. list", testee.getList()[1], Reference(Reference::Ship, 50));
    a.checkEqual("04. list", testee.getList()[2], Reference(Reference::Ship, 80));

    a.check("11. isCurrent",        !testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* Normal behaviour, next-turn ships, starting from ship. */
AFL_TEST("game.proxy.ListProxy:buildNext:from-ship", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    // Those shall be listed:
    addShip(t, 20, OWNER, X+10, Y, Object::Playable).setWaypoint(Point(X, Y));
    addShip(t, 50, OWNER, X, Y-10, Object::Playable).setWaypoint(Point(X, Y));
    addShip(t, 80, OWNER, X-10, Y, Object::Playable).setWaypoint(Point(X, Y));

    testee.buildNext(ind, Point(0, 0), 50, List::Options_t());

    a.checkEqual("01. list", testee.getList().size(), 3U);
    a.checkEqual("02. list", testee.getList()[0], Reference(Reference::Ship, 20));
    a.checkEqual("03. list", testee.getList()[1], Reference(Reference::Ship, 50));
    a.checkEqual("04. list", testee.getList()[2], Reference(Reference::Ship, 80));

    a.check("11. isCurrent",        !testee.isCurrent());
    a.check("12. isUniquePlayable", !testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* Normal behaviour, next-turn ships. */
AFL_TEST("game.proxy.ListProxy:buildNext:unique", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    addShip(t, 20, OWNER, X+10, Y, Object::Playable).setWaypoint(Point(X, Y));

    testee.buildNext(ind, Point(X, Y), 0, List::Options_t());

    a.checkEqual("01. list", testee.getList().size(), 1U);
    a.checkEqual("02. list", testee.getList()[0], Reference(Reference::Ship, 20));

    a.check("11. isCurrent",        !testee.isCurrent());
    a.check("12. isUniquePlayable",  testee.isUniquePlayable());
    a.check("13. hasExcludedShip",  !testee.hasExcludedShip());
    a.check("14. hasHidingPlanet",  !testee.hasHidingPlanet());
}

/* Forwarding of CPEnableRemote=No */
AFL_TEST("game.proxy.ListProxy:buildNext:remote:off", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t).hostConfiguration()[HostConfiguration::CPEnableRemote].set(0);

    testee.buildNext(ind, Point(X, Y), 0, List::Options_t());

    a.check("", !testee.hasRemoteControl());
}

/* Forwarding of CPEnableRemote=Yes */
AFL_TEST("game.proxy.ListProxy:buildNext:remote:on", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t).hostConfiguration()[HostConfiguration::CPEnableRemote].set(1);

    testee.buildNext(ind, Point(X, Y), 0, List::Options_t());

    a.check("", testee.hasRemoteControl());
}

/* Cargo summary. */
AFL_TEST("game.proxy.ListProxy:buildNext:getCargoSummary", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::ListProxy testee(t.gameSender());

    addShipList(t);
    addRoot(t);

    // First ship does alchemy, to prove that we're getting predicted data
    Ship& sh = addShip(t, 20, OWNER, X+10, Y, Object::Playable);
    sh.setCargo(Element::Tritanium, 1);
    sh.setCargo(Element::Duranium, 2);
    sh.setCargo(Element::Molybdenum, 3);
    sh.setCargo(Element::Supplies, 90);
    sh.setWaypoint(Point(X, Y));
    sh.addShipSpecialFunction(t.session().getShipList()->modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::MerlinAlchemy));
    sh.setName("Orville");

    Ship& sh2 = addShip(t, 30, OWNER, X, Y, Object::Playable);
    sh2.setName("Serenity");

    testee.buildNext(ind, Point(X, Y), 0, List::Options_t());
    CostSummary sum = testee.getCargoSummary(ind);

    a.checkEqual("01. size", sum.getNumItems(), 2U);
    a.checkEqual("02. id",   sum.get(0)->id, 20);
    a.checkEqual("03. Tri",  sum.get(0)->cost.get(Cost::Tritanium), 11);
    a.checkEqual("04. Dur",  sum.get(0)->cost.get(Cost::Duranium), 12);
    a.checkEqual("05. Mol",  sum.get(0)->cost.get(Cost::Molybdenum), 13);
    a.checkEqual("06. id",   sum.get(0)->name, "Ship #20: Orville");
    a.checkEqual("07. id",   sum.get(1)->name, "Ship #30: Serenity");
}
