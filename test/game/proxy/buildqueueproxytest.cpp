/**
  *  \file test/game/proxy/buildqueueproxytest.cpp
  *  \brief Test for game::proxy::BuildQueueProxy
  */

#include "game/proxy/buildqueueproxy.hpp"

#include "afl/test/testrunner.hpp"
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

    Planet& addPlanet(afl::test::Assert a, game::test::SessionThread& s, game::Id_t planetId, String_t fc)
    {
        Planet* p = s.session().getGame()->currentTurn().universe().planets().create(planetId);
        a.checkNonNull("planet created", p);

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

        p->internalCheck(game::map::Configuration(), game::PlayerSet_t(PLAYER_NR), 15, s.session().translator(), s.session().log());
        p->setPlayability(game::map::Object::Playable);

        return *p;
    }

    void init(afl::test::Assert a, game::test::SessionThread& s)
    {
        initRoot(s);
        initShipList(s);
        initGame(s);

        addPlanet(a("p1"), s, 1, "xyz");
        addPlanet(a("p2"), s, 2, "PB3");
        addPlanet(a("p3"), s, 3, "PB1");
        addPlanet(a("p4"), s, 4, "abc");
    }

    void initScore(game::test::SessionThread& s)
    {
        game::Game* g = s.session().getGame().get();
        g->currentTurn().setTurnNumber(77);

        game::score::TurnScore::Slot_t slot = g->scores().addSlot(game::score::ScoreId_Bases);
        game::score::TurnScore& t = g->scores().addTurn(77, game::Timestamp(2000, 12, 24, 12, 0, 0));

        t.set(slot, PLAYER_NR+1, 3);
        t.set(slot, PLAYER_NR+2, 7);
        t.set(slot, PLAYER_NR,   5);
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
AFL_TEST("game.proxy.BuildQueueProxy:init", a)
{
    // Environment
    game::test::SessionThread s;
    init(a, s);
    initScore(s);

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Get initial status
    game::proxy::BuildQueueProxy::Infos_t data;
    game::proxy::BuildQueueProxy::GlobalInfo global;
    testee.getStatus(ind, data, global);
    a.checkEqual("01. size", data.size(), 4U);
    a.checkEqual("02. planetId", data[0].planetId, 3);
    a.checkEqual("03. planetId", data[1].planetId, 2);
    a.checkEqual("04. planetId", data[2].planetId, 1);
    a.checkEqual("05. planetId", data[3].planetId, 4);
    a.checkEqual("06. planetId", data[0].friendlyCode, "PB1");
    a.checkEqual("07. planetId", data[1].friendlyCode, "PB3");
    a.checkEqual("08. planetId", data[2].friendlyCode, "xyz");
    a.checkEqual("09. planetId", data[3].friendlyCode, "abc");
    a.checkEqual("10. numBases", global.numBases, 4);
    a.checkEqual("11. totalBases", global.totalBases, 15);
}

/** Test increasePriority().
    A: prepare a universe. Call increasePriority().
    E: correct status returned by getStatus(). */
AFL_TEST("game.proxy.BuildQueueProxy:increasePriority", a)
{
    // Environment
    game::test::SessionThread s;
    init(a, s);

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Increase 'abc'
    testee.increasePriority(3);
    game::proxy::BuildQueueProxy::Infos_t data;
    game::proxy::BuildQueueProxy::GlobalInfo global;
    testee.getStatus(ind, data, global);
    a.checkEqual("01. size", data.size(), 4U);
    a.checkEqual("02. planetId", data[0].planetId, 3);
    a.checkEqual("03. planetId", data[1].planetId, 2);
    a.checkEqual("04. planetId", data[2].planetId, 4);
    a.checkEqual("05. planetId", data[3].planetId, 1);
    a.checkEqual("06. planetId", data[0].friendlyCode, "PB1");
    a.checkEqual("07. planetId", data[1].friendlyCode, "PB3");
    a.checkEqual("08. planetId", data[2].friendlyCode, "PB4");
    a.checkEqual("09. planetId", data[3].friendlyCode, "xyz");
}

/** Test decreasePriority().
    A: prepare a universe. Call decreasePriority().
    E: correct status returned by getStatus(). */
AFL_TEST("game.proxy.BuildQueueProxy:decreasePriority", a)
{
    // Environment
    game::test::SessionThread s;
    init(a, s);

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Decrease 'PB1'
    testee.decreasePriority(0);
    game::proxy::BuildQueueProxy::Infos_t data;
    game::proxy::BuildQueueProxy::GlobalInfo global;
    testee.getStatus(ind, data, global);
    a.checkEqual("01. size", data.size(), 4U);
    a.checkEqual("02. planetId", data[0].planetId, 3);
    a.checkEqual("03. planetId", data[1].planetId, 2);
    a.checkEqual("04. planetId", data[2].planetId, 1);
    a.checkEqual("05. planetId", data[3].planetId, 4);
    a.checkEqual("06. planetId", data[0].friendlyCode, "PB2");
    a.checkEqual("07. planetId", data[1].friendlyCode, "PB3");
    a.checkEqual("08. planetId", data[2].friendlyCode, "xyz");
    a.checkEqual("09. planetId", data[3].friendlyCode, "abc");
}

/** Test setPriority().
    A: prepare a universe. Call setPriority().
    E: correct status returned by getStatus(). */
AFL_TEST("game.proxy.BuildQueueProxy:setPriority", a)
{
    // Environment
    game::test::SessionThread s;
    init(a, s);

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Change abc -> 2
    testee.setPriority(3, 2);
    game::proxy::BuildQueueProxy::Infos_t data;
    game::proxy::BuildQueueProxy::GlobalInfo global;
    testee.getStatus(ind, data, global);
    a.checkEqual("01. size", data.size(), 4U);
    a.checkEqual("02. planetId", data[0].planetId, 3);
    a.checkEqual("03. planetId", data[1].planetId, 4);
    a.checkEqual("04. planetId", data[2].planetId, 2);
    a.checkEqual("05. planetId", data[3].planetId, 1);
    a.checkEqual("06. planetId", data[0].friendlyCode, "PB1");
    a.checkEqual("07. planetId", data[1].friendlyCode, "PB2");
    a.checkEqual("08. planetId", data[2].friendlyCode, "PB3");
    a.checkEqual("09. planetId", data[3].friendlyCode, "xyz");
}

/** Test status signalisation.
    A: prepare a universe. Connect a listener. Call a modifier method.
    E: correct status reported on listener. */
AFL_TEST("game.proxy.BuildQueueProxy:signal", a)
{
    // Environment
    game::test::SessionThread s;
    init(a, s);

    // Testee
    util::SimpleRequestDispatcher disp;
    game::proxy::BuildQueueProxy testee(s.gameSender(), disp);

    UpdateReceiver recv;
    testee.sig_update.add(&recv, &UpdateReceiver::onUpdate);

    // Set priority. This should trigger an update.
    testee.setPriority(3, 2);
    while (recv.result.empty()) {
        a.check("01. wait", disp.wait(100));
    }

    // Verify
    a.checkEqual("11. size", recv.result.size(), 4U);
    a.checkEqual("12. friendlyCode", recv.result[0].friendlyCode, "PB1");
    a.checkEqual("13. friendlyCode", recv.result[1].friendlyCode, "PB2");
    a.checkEqual("14. friendlyCode", recv.result[2].friendlyCode, "PB3");
    a.checkEqual("15. friendlyCode", recv.result[3].friendlyCode, "xyz");
}

/** Test commit().
    A: prepare a universe. Call a modifier method.
    E: correct update applied to universe. */
AFL_TEST("game.proxy.BuildQueueProxy:commit", a)
{
    // Environment
    game::test::SessionThread s;
    init(a, s);

    // Testee
    util::SimpleRequestDispatcher disp;
    game::proxy::BuildQueueProxy testee(s.gameSender(), disp);

    // Set priority. This should trigger an update.
    testee.setPriority(3, 2);
    testee.commit();
    s.sync();

    // Verify
    Planet& p = *s.session().getGame()->currentTurn().universe().planets().get(4);
    a.checkEqual("getFriendlyCode", p.getFriendlyCode().orElse(""), "PB2");
}

/** Test behaviour on empty universe.
    A: prepare empty universe. Call getStatus().
    E: empty status returned. */
AFL_TEST("game.proxy.BuildQueueProxy:empty", a)
{
    // Empty Environment
    game::test::SessionThread s;

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::BuildQueueProxy testee(s.gameSender(), ind);

    // Get initial status
    game::proxy::BuildQueueProxy::Infos_t data;
    game::proxy::BuildQueueProxy::GlobalInfo global;
    testee.getStatus(ind, data, global);
    a.checkEqual("01. size", data.size(), 0U);
    a.checkEqual("02. numBases", global.numBases, 0);
    a.checkEqual("03. totalBases", global.totalBases, 0);
}
