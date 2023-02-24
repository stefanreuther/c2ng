/**
  *  \file u/t_game_proxy_shipspeedproxy.cpp
  *  \brief Test for game::proxy::ShipSpeedProxy
  */

#include "game/proxy/shipspeedproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using afl::base::Ptr;
using game::Game;
using game::HostVersion;
using game::Root;
using game::Session;
using game::map::Ship;
using game::map::Universe;
using game::spec::ShipList;

namespace {
    const int HULL_NR = 3;
    const int SHIP_NR = 47;
    const int OWNER = 5;

    void addShipList(Session& s)
    {
        Ptr<ShipList> shipList = new ShipList();
        shipList->hulls().create(HULL_NR);
        s.setShipList(shipList);
    }

    void addRoot(Session& s)
    {
        Ptr<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,2,0))).asPtr();
        s.setRoot(root);
    }
}

/** Test error behaviour: empty turn.
    A: make empty session. Query ship state.
    E: must report maxSpeed=0 */
void
TestGameProxyShipSpeedProxy::testEmpty()
{
    // Make empty session
    game::test::SessionThread h;

    // Query ship state
    game::test::WaitIndicator ind;
    game::proxy::ShipSpeedProxy testee(h.gameSender(), 99);

    game::proxy::ShipSpeedProxy::Status result = testee.getStatus(ind);

    // Verify
    TS_ASSERT_EQUALS(result.currentSpeed, 0);
    TS_ASSERT_EQUALS(result.maxSpeed, 0);          // indicates ship Id was invalid
}

/** Test normal behaviour.
    A: make a session with a single ship in it. Query and change ship state.
    E: must report correct state; must correctly update speed. */
void
TestGameProxyShipSpeedProxy::testSimple()
{
    // Environment
    // - session
    game::test::SessionThread h;
    addShipList(h.session());
    addRoot(h.session());

    // - add a turn with a ship
    Ptr<Game> g = new Game();
    Ship* sh = g->currentTurn().universe().ships().create(SHIP_NR);
    sh->addShipXYData(game::map::Point(1, 2), OWNER, 444, game::PlayerSet_t(OWNER));
    sh->internalCheck(game::PlayerSet_t(OWNER), 15);
    sh->setPlayability(game::map::Object::ReadOnly);
    sh->setWarpFactor(3);
    sh->setHull(HULL_NR);
    h.session().setGame(g);

    // Test subject
    game::proxy::ShipSpeedProxy testee(h.gameSender(), SHIP_NR);

    // - query ship state
    game::test::WaitIndicator ind;
    game::proxy::ShipSpeedProxy::Status result = testee.getStatus(ind);
    TS_ASSERT_EQUALS(result.currentSpeed, 3);
    TS_ASSERT_EQUALS(result.maxSpeed, 9);

    // - change speed
    testee.setSpeed(7);

    // - query state again (also required for synchronisation)
    result = testee.getStatus(ind);
    TS_ASSERT_EQUALS(result.currentSpeed, 7);

    // - verify ship
    TS_ASSERT_EQUALS(sh->getWarpFactor().orElse(-1), 7);
}


/** Test hyperjump behaviour.
    A: make a session with a fleet with multiple ships. Query and change ship state.
    E: must report correct state; must correctly update speed. */
void
TestGameProxyShipSpeedProxy::testHyper()
{
    // Environment
    // - session
    game::test::SessionThread h;
    addShipList(h.session());
    addRoot(h.session());

    // - add a turn with two ships
    Ptr<Game> g = new Game();
    for (int id = SHIP_NR; id < SHIP_NR+2; ++id) {
        Ship* sh = g->currentTurn().universe().ships().create(id);
        sh->addShipXYData(game::map::Point(1, 2), OWNER, 444, game::PlayerSet_t(OWNER+1));
        sh->internalCheck(game::PlayerSet_t(OWNER+1), 15);
        sh->setPlayability(game::map::Object::ReadOnly);
        sh->setWarpFactor(3);
        sh->setHull(HULL_NR);
        sh->setFleetNumber(SHIP_NR);
        sh->addShipSpecialFunction(game::spec::BasicHullFunction::Hyperdrive);
        sh->setFriendlyCode(String_t("abc"));
    }
    h.session().setGame(g);

    // Test subject
    game::proxy::ShipSpeedProxy testee(h.gameSender(), SHIP_NR);

    // - query ship state
    game::test::WaitIndicator ind;
    game::proxy::ShipSpeedProxy::Status result = testee.getStatus(ind);
    TS_ASSERT_EQUALS(result.currentSpeed, 3);
    TS_ASSERT_EQUALS(result.maxSpeed, 10);          // indicates HYP capability

    // - change speed
    testee.setSpeed(result.maxSpeed);

    // - query state again (also required for synchronisation)
    result = testee.getStatus(ind);
    TS_ASSERT_EQUALS(result.currentSpeed, 10);

    // - verify ship
    Universe& univ = g->currentTurn().universe();
    TS_ASSERT_EQUALS(univ.ships().get(SHIP_NR)->getFriendlyCode().orElse(""), "HYP");
    TS_ASSERT_EQUALS(univ.ships().get(SHIP_NR+1)->getFriendlyCode().orElse(""), "HYP");

    // - change back
    testee.setSpeed(1);
    testee.getStatus(ind);

    TS_ASSERT_EQUALS(univ.ships().get(SHIP_NR)->getFriendlyCode().orElse(""), "abc");
    TS_ASSERT_EQUALS(univ.ships().get(SHIP_NR)->getWarpFactor().orElse(-1), 1);
    TS_ASSERT_EQUALS(univ.ships().get(SHIP_NR+1)->getFriendlyCode().orElse(""), "abc");
    TS_ASSERT_EQUALS(univ.ships().get(SHIP_NR+1)->getWarpFactor().orElse(-1), 1);
}

