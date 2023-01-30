/**
  *  \file u/t_game_proxy_buildqueueproxy.cpp
  *  \brief Test for game::proxy::BuildQueueProxy
  */

#include "game/proxy/buildqueueproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "util/simplerequestdispatcher.hpp"

using afl::base::Ptr;
using game::map::Planet;
using game::Game;
using game::HostVersion;

namespace {
    const int PLAYER_NR = 4;
    const int HULL_NR = 1;

    void initRoot(game::test::SessionThread& s)
    {
        s.session().setRoot(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,1,0))).asPtr());
    }

    void initShipList(game::test::SessionThread& s)
    {
        using game::spec::ShipList;
        using game::spec::Hull;

        Ptr<ShipList> shipList = new ShipList();
        Hull* pHull = shipList->hulls().create(HULL_NR);
        pHull->setName("Boat");
        pHull->setMass(100);
        pHull->setNumEngines(1);
        shipList->hullAssignments().add(PLAYER_NR, HULL_NR, HULL_NR);

        s.session().setShipList(shipList);
    }

    void initGame(game::test::SessionThread& s)
    {
        Ptr<Game> g = new Game();
        g->setViewpointPlayer(PLAYER_NR);
        s.session().setGame(g);
    }

    Planet& addPlanet(game::test::SessionThread& s, game::Id_t planetId, String_t fc)
    {
        Planet* p = s.session().getGame()->currentTurn().universe().planets().create(planetId);
        TS_ASSERT(p != 0);

        game::map::PlanetData pd;
        pd.owner = PLAYER_NR;
        pd.colonistClans = 100;
        pd.friendlyCode = fc;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER_NR));
        p->setPosition(game::map::Point(1000 + planetId, 2000));

        game::map::BaseData bd;
        bd.shipBuildOrder.setHullIndex(1);
        bd.shipBuildOrder.setEngineType(1);
        bd.hullStorage.set(1, 100);
        bd.engineStorage.set(1, 100);
        p->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));

        p->internalCheck(game::map::Configuration(), s.session().translator(), s.session().log());
        p->setPlayability(game::map::Object::Playable);

        return *p;
    }

    void init(game::test::SessionThread& s)
    {
        initRoot(s);
        initShipList(s);
        initGame(s);

        addPlanet(s, 1, "xyz");
        addPlanet(s, 2, "PB3");
        addPlanet(s, 3, "PB1");
        addPlanet(s, 4, "abc");
    }

    struct UpdateReceiver {
        game::proxy::BuildQueueProxy::Infos_t result;

        void onUpdate(const game::proxy::BuildQueueProxy::Infos_t& proxy)
            { result = proxy; }
    };
}

/** Test initialisation.
    A: prepare a universe. Call getStatus().
    E: correct status returned. */
void
TestGameProxyBuildQueueProxy::testInit()
{
    // Environment
    game::test::SessionThread s;
    init(s);

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Get initial status
    game::proxy::BuildQueueProxy::Infos_t data;
    testee.getStatus(ind, data);
    TS_ASSERT_EQUALS(data.size(), 4U);
    TS_ASSERT_EQUALS(data[0].planetId, 3);
    TS_ASSERT_EQUALS(data[1].planetId, 2);
    TS_ASSERT_EQUALS(data[2].planetId, 1);
    TS_ASSERT_EQUALS(data[3].planetId, 4);
    TS_ASSERT_EQUALS(data[0].friendlyCode, "PB1");
    TS_ASSERT_EQUALS(data[1].friendlyCode, "PB3");
    TS_ASSERT_EQUALS(data[2].friendlyCode, "xyz");
    TS_ASSERT_EQUALS(data[3].friendlyCode, "abc");
}

/** Test increasePriority().
    A: prepare a universe. Call increasePriority().
    E: correct status returned by getStatus(). */
void
TestGameProxyBuildQueueProxy::testIncrease()
{
    // Environment
    game::test::SessionThread s;
    init(s);

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Increase 'abc'
    testee.increasePriority(3);
    game::proxy::BuildQueueProxy::Infos_t data;
    testee.getStatus(ind, data);
    TS_ASSERT_EQUALS(data.size(), 4U);
    TS_ASSERT_EQUALS(data[0].planetId, 3);
    TS_ASSERT_EQUALS(data[1].planetId, 2);
    TS_ASSERT_EQUALS(data[2].planetId, 4);
    TS_ASSERT_EQUALS(data[3].planetId, 1);
    TS_ASSERT_EQUALS(data[0].friendlyCode, "PB1");
    TS_ASSERT_EQUALS(data[1].friendlyCode, "PB3");
    TS_ASSERT_EQUALS(data[2].friendlyCode, "PB4");
    TS_ASSERT_EQUALS(data[3].friendlyCode, "xyz");
}

/** Test decreasePriority().
    A: prepare a universe. Call decreasePriority().
    E: correct status returned by getStatus(). */
void
TestGameProxyBuildQueueProxy::testDecrease()
{
    // Environment
    game::test::SessionThread s;
    init(s);

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Decrease 'PB1'
    testee.decreasePriority(0);
    game::proxy::BuildQueueProxy::Infos_t data;
    testee.getStatus(ind, data);
    TS_ASSERT_EQUALS(data.size(), 4U);
    TS_ASSERT_EQUALS(data[0].planetId, 3);
    TS_ASSERT_EQUALS(data[1].planetId, 2);
    TS_ASSERT_EQUALS(data[2].planetId, 1);
    TS_ASSERT_EQUALS(data[3].planetId, 4);
    TS_ASSERT_EQUALS(data[0].friendlyCode, "PB2");
    TS_ASSERT_EQUALS(data[1].friendlyCode, "PB3");
    TS_ASSERT_EQUALS(data[2].friendlyCode, "xyz");
    TS_ASSERT_EQUALS(data[3].friendlyCode, "abc");
}

/** Test setPriority().
    A: prepare a universe. Call setPriority().
    E: correct status returned by getStatus(). */
void
TestGameProxyBuildQueueProxy::testSet()
{
    // Environment
    game::test::SessionThread s;
    init(s);

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Change abc -> 2
    testee.setPriority(3, 2);
    game::proxy::BuildQueueProxy::Infos_t data;
    testee.getStatus(ind, data);
    TS_ASSERT_EQUALS(data.size(), 4U);
    TS_ASSERT_EQUALS(data[0].planetId, 3);
    TS_ASSERT_EQUALS(data[1].planetId, 4);
    TS_ASSERT_EQUALS(data[2].planetId, 2);
    TS_ASSERT_EQUALS(data[3].planetId, 1);
    TS_ASSERT_EQUALS(data[0].friendlyCode, "PB1");
    TS_ASSERT_EQUALS(data[1].friendlyCode, "PB2");
    TS_ASSERT_EQUALS(data[2].friendlyCode, "PB3");
    TS_ASSERT_EQUALS(data[3].friendlyCode, "xyz");
}

/** Test status signalisation.
    A: prepare a universe. Connect a listener. Call a modifier method.
    E: correct status reported on listener. */
void
TestGameProxyBuildQueueProxy::testSignal()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    game::test::SessionThread s;
    init(s);

    // Testee
    util::SimpleRequestDispatcher disp;
    game::proxy::BuildQueueProxy testee(s.gameSender(), disp);

    UpdateReceiver recv;
    testee.sig_update.add(&recv, &UpdateReceiver::onUpdate);

    // Set priority. This should trigger an update.
    testee.setPriority(3, 2);
    while (recv.result.empty()) {
        TS_ASSERT(disp.wait(100));
    }

    // Verify
    TS_ASSERT_EQUALS(recv.result.size(), 4U);
    TS_ASSERT_EQUALS(recv.result[0].friendlyCode, "PB1");
    TS_ASSERT_EQUALS(recv.result[1].friendlyCode, "PB2");
    TS_ASSERT_EQUALS(recv.result[2].friendlyCode, "PB3");
    TS_ASSERT_EQUALS(recv.result[3].friendlyCode, "xyz");
}

/** Test commit().
    A: prepare a universe. Call a modifier method.
    E: correct update applied to universe. */
void
TestGameProxyBuildQueueProxy::testCommit()
{
    // Environment
    game::test::SessionThread s;
    init(s);

    // Testee
    util::SimpleRequestDispatcher disp;
    game::proxy::BuildQueueProxy testee(s.gameSender(), disp);

    // Set priority. This should trigger an update.
    testee.setPriority(3, 2);
    testee.commit();
    s.sync();

    // Verify
    Planet& p = *s.session().getGame()->currentTurn().universe().planets().get(4);
    TS_ASSERT_EQUALS(p.getFriendlyCode().orElse(""), "PB2");
}

/** Test behaviour on empty universe.
    A: prepare empty universe. Call getStatus().
    E: empty status returned. */
void
TestGameProxyBuildQueueProxy::testEmpty()
{
    // Empty Environment
    game::test::SessionThread s;

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Get initial status
    game::proxy::BuildQueueProxy::Infos_t data;
    testee.getStatus(ind, data);
    TS_ASSERT_EQUALS(data.size(), 0U);
}

