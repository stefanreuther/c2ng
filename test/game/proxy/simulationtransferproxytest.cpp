/**
  *  \file test/game/proxy/simulationtransferproxytest.cpp
  *  \brief Test for game::proxy::SimulationTransferProxy
  */

#include "game/proxy/simulationtransferproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/sessionextra.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using game::Reference;
using game::proxy::SimulationSetupProxy;
using game::proxy::SimulationTransferProxy;
using game::test::Counter;
using game::test::SessionThread;
using game::test::WaitIndicator;

namespace {
    void prepare(SessionThread& thread)
    {
        // Shiplist
        afl::base::Ptr<game::spec::ShipList> list = new game::spec::ShipList();
        game::test::initStandardBeams(*list);
        game::test::initStandardTorpedoes(*list);
        game::test::addOutrider(*list);
        game::test::addTranswarp(*list);
        thread.session().setShipList(list);

        // Root
        afl::base::Ptr<game::Root> root = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4, 0, 0))).asPtr();
        thread.session().setRoot(root);

        // Game
        afl::base::Ptr<game::Game> g = new game::Game();
        thread.session().setGame(g);
    }

    game::map::Ship& addShip(SessionThread& thread, int shipId)
    {
        game::map::Universe& univ = thread.session().getGame()->currentTurn().universe();

        game::map::ShipData sd;
        sd.owner = 1;
        sd.hullType = game::test::OUTRIDER_HULL_ID;
        sd.x = 2000;
        sd.y = 2000;
        sd.engineType = 9;
        sd.beamType = 7;
        sd.numBeams = 1;
        sd.torpedoType = 0;
        sd.numLaunchers = 0;
        sd.ammo = 0;
        sd.friendlyCode = "abc";
        sd.name = "The Ship";

        game::map::Ship* sh = univ.ships().create(shipId);
        sh->addCurrentShipData(sd, game::PlayerSet_t(1));
        sh->internalCheck(game::PlayerSet_t(1), 10);
        sh->setPlayability(game::map::Object::Playable);

        return *sh;
    }
    game::map::Planet& addPlanet(SessionThread& thread, int planetId)
    {
        game::map::Universe& univ = thread.session().getGame()->currentTurn().universe();

        game::map::Planet* pl = univ.planets().create(planetId);
        pl->setPosition(game::map::Point(2000, 2000));
        pl->setOwner(2);
        pl->setFriendlyCode(String_t("abc"));
        pl->setName("The Planet");

        pl->internalCheck(thread.session().getGame()->mapConfiguration(), game::PlayerSet_t(1), 10, thread.session().translator(), thread.session().log());
        pl->setPlayability(game::map::Object::Playable);

        return *pl;
    }
}

/** Test behaviour on empty session.
    A: create empty session. Create SimulationTransferProxy. Call copyObjectFromGame() with various references.
    E: must return failure */
AFL_TEST("game.proxy.SimulationTransferProxy:empty", a)
{
    SessionThread thread;
    WaitIndicator ind;
    SimulationTransferProxy t(thread.gameSender());

    a.checkEqual("01", t.copyObjectFromGame(ind, Reference()), false);
    a.checkEqual("02", t.copyObjectFromGame(ind, Reference(Reference::Ship, 5)), false);
    a.checkEqual("03", t.copyObjectFromGame(ind, Reference(Reference::Planet, 10)), false);
}

/** Test normal case, ship.
    A: create session with shiplist and ship. Create SimulationTransferProxy. Call copyObjectFromGame() with valid reference.
    E: must return success. Must create correct ship in simulation. */
AFL_TEST("game.proxy.SimulationTransferProxy:ship", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    game::map::Ship& sh = addShip(thread, 77);
    SimulationTransferProxy t(thread.gameSender());

    const Reference REF(Reference::Ship, 77);

    // Ship not in simulation
    a.checkEqual("01. hasObject", t.hasObject(ind, REF), false);

    // Add ship
    a.checkEqual("11. copyObjectFromGame", t.copyObjectFromGame(ind, REF), true);

    // Verify content of simulation
    game::sim::Setup& setup = game::sim::getSimulatorSession(thread.session())->setup();
    a.checkEqual("21. getNumShips", setup.getNumShips(), 1U);
    a.checkEqual("22. getFriendlyCode", setup.getShip(0)->getFriendlyCode(), "abc");
    a.checkEqual("23. hasObject", t.hasObject(ind, REF), true);

    // Modify and add again
    sh.setFriendlyCode(String_t("foo"));
    a.checkEqual("31. copyObjectFromGame", t.copyObjectFromGame(ind, REF), true);
    a.checkEqual("32. getFriendlyCode", setup.getShip(0)->getFriendlyCode(), "foo");
}

/** Test normal case, planet.
    A: create session with planet. Create SimulationTransferProxy. Call copyObjectFromGame() with valid reference.
    E: must return success. Must create correct ship in simulation. */
AFL_TEST("game.proxy.SimulationTransferProxy:planet", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    game::map::Planet& sh = addPlanet(thread, 135);
    SimulationTransferProxy t(thread.gameSender());

    const Reference REF(Reference::Planet, 135);

    // Planet not in simulation
    a.checkEqual("01. hasObject", t.hasObject(ind, REF), false);

    // Add planet
    a.checkEqual("11. copyObjectFromGame", t.copyObjectFromGame(ind, REF), true);

    // Verify content of simulation
    game::sim::Setup& setup = game::sim::getSimulatorSession(thread.session())->setup();
    a.checkEqual("21. getFriendlyCode", setup.getPlanet()->getFriendlyCode(), "abc");
    a.checkEqual("22. hasObject", t.hasObject(ind, REF), true);

    // Modify and add again
    sh.setFriendlyCode(String_t("bar"));
    a.checkEqual("31. copyObjectFromGame", t.copyObjectFromGame(ind, REF), true);
    a.checkEqual("32. getFriendlyCode", setup.getPlanet()->getFriendlyCode(), "bar");
}


/** Test copyObjectsFromGame().
    A: create session with shiplist and some ship. Create SimulationTransferProxy. Call copyObjectsFromGame() with a list containing valid and invalid references.
    E: must return correct number of units copied. */
AFL_TEST("game.proxy.SimulationTransferProxy:copyObjectsFromGame", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    addShip(thread, 1);
    addShip(thread, 5);
    addShip(thread, 17);
    addPlanet(thread, 333);
    SimulationTransferProxy t(thread.gameSender());

    game::ref::List list;
    list.add(Reference(Reference::Ship, 1));
    list.add(Reference(Reference::Ship, 3));      // invalid
    list.add(Reference(Reference::Hull, 5));      // invalid
    list.add(Reference(Reference::Planet, 7));    // invalid
    list.add(Reference(Reference::Starbase, 333));
    list.add(Reference(Reference::Ship, 5));
    list.add(Reference());                        // invalid

    // Add units
    size_t n = t.copyObjectsFromGame(ind, list);
    a.checkEqual("01. n", n, 3U);

    // Verify content of simulation
    game::sim::Setup& setup = game::sim::getSimulatorSession(thread.session())->setup();
    a.checkEqual("11. getNumShips", setup.getNumShips(), 2U);
    a.checkNonNull("12. getPlanet", setup.getPlanet());
}

/** Test interaction with SimulationSetupProxy.
    A: create session with shiplist and ship. Create SimulationTransferProxy. Call copyObjectFromGame() with valid reference.
    E: must return success. Must provide callback on SimulationSetupProxy. */
AFL_TEST("game.proxy.SimulationTransferProxy:interaction", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    game::map::Ship& sh = addShip(thread, 77);
    SimulationTransferProxy t(thread.gameSender());
    SimulationSetupProxy sp(thread.gameSender(), ind);

    const Reference REF(Reference::Ship, 77);

    // Observer SimulationSetupProxy's signals
    Counter onListChange;
    sp.sig_listChange.add(&onListChange, &Counter::increment);

    Counter onObjectChange;
    sp.sig_objectChange.add(&onObjectChange, &Counter::increment);

    // Add object. Must create update on sig_listChange.
    int numListChanges = onListChange.get();
    a.checkEqual("01. copyObjectFromGame", t.copyObjectFromGame(ind, REF), true);

    thread.sync();
    ind.processQueue();
    a.check("11. onListChange", onListChange.get() > numListChanges);

    // Observe object. This will generate an immediate callback as per SimulationSetupProxy's specs.
    sp.setSlot(0);

    thread.sync();
    ind.processQueue();
    int numObjectChanges = onObjectChange.get();
    a.check("21. onListChange", numObjectChanges > 0); // SimulationSetupProxy guarantee

    // Modify object
    sh.setFriendlyCode(String_t("baz"));
    a.checkEqual("31. copyObjectFromGame", t.copyObjectFromGame(ind, REF), true);

    thread.sync();
    ind.processQueue();
    a.check("41. onListChange", onObjectChange.get() > numObjectChanges);
}
