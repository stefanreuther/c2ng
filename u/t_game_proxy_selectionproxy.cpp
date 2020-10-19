/**
  *  \file u/t_game_proxy_selectionproxy.cpp
  *  \brief Test for game::proxy::SelectionProxy
  */

#include "game/proxy/selectionproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "util/simplerequestdispatcher.hpp"

using afl::base::Ptr;
using game::Game;
using game::map::Planet;
using game::map::Ship;
using game::map::Universe;
using game::proxy::SelectionProxy;
using game::test::SessionThread;
using game::test::WaitIndicator;
using util::SimpleRequestDispatcher;

namespace {
    Planet& createPlanet(Universe& u, game::Id_t id)
    {
        Planet& p = *u.planets().create(id);
        p.setPosition(game::map::Point(1000, 1000+id));

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p.internalCheck(game::map::Configuration(), tx, log);
        p.setPlayability(game::map::Object::NotPlayable);
        return p;
    }

    Ship& createShip(Universe& u, game::Id_t id)
    {
        Ship& s = *u.ships().create(id);
        s.addShipXYData(game::map::Point(1000, 1000+id), 3, 222, game::PlayerSet_t(1));
        s.internalCheck();
        s.setPlayability(game::map::Object::NotPlayable);
        return s;
    }

    void prepare(SessionThread& h)
    {
        Ptr<Game> g = new Game();

        // Universe with some marked units
        Universe& univ = g->currentTurn().universe();
        createPlanet(univ, 10).setIsMarked(true);
        createPlanet(univ, 20);
        createPlanet(univ, 30);
        createShip(univ, 11);
        createShip(univ, 12).setIsMarked(true);
        createShip(univ, 13);
        createShip(univ, 14).setIsMarked(true);
        createShip(univ, 15);

        // Some marked units in layer 3
        g->selections().get(game::map::Selections::Planet, 3)->set(20, true);
        g->selections().get(game::map::Selections::Ship,   3)->set(13, true);

        h.session().setGame(g);
    }

    struct ChangeReceiver {
        std::vector<SelectionProxy::Info> infos;

        void onSelectionChange(const SelectionProxy::Info& info)
            { infos.push_back(info); }
    };
}

/** Test use on empty session.
    A: create empty session.
    E: synchronous method calls must successfully execute */
void
TestGameProxySelectionProxy::testEmpty()
{
    // Environment
    SessionThread h;
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call init()
    SelectionProxy::Info info;
    TS_ASSERT_THROWS_NOTHING(t.init(ind, info));
    TS_ASSERT_EQUALS(info.currentLayer, 0U);
    TS_ASSERT(info.layers.empty());

    // Call executeExpression
    // (we don't care whether this is reported as error or not, but it must not throw or hang)
    String_t error;
    TS_ASSERT_THROWS_NOTHING(t.executeExpression(ind, "A", 1, error));
}

/** Test normal initialisation.
    A: create session with some selections.
    E: init() must report correct result. */
void
TestGameProxySelectionProxy::testInit()
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call init()
    SelectionProxy::Info info;
    TS_ASSERT_THROWS_NOTHING(t.init(ind, info));
    TS_ASSERT_EQUALS(info.currentLayer, 0U);
    TS_ASSERT(info.layers.size() > 3U);
    TS_ASSERT_EQUALS(info.layers[0].numPlanets, 1U);
    TS_ASSERT_EQUALS(info.layers[0].numShips, 2U);
    TS_ASSERT_EQUALS(info.layers[3].numPlanets, 1U);
    TS_ASSERT_EQUALS(info.layers[3].numShips, 1U);
}

/** Test signalisation of changes, external.
    A: create session with some selections. Initiate change on game side.
    E: change must be reflected to UI side. */
void
TestGameProxySelectionProxy::testSignalExternal()
{
    // Environment
    SessionThread h;
    prepare(h);
    SimpleRequestDispatcher disp;
    SelectionProxy t(h.gameSender(), disp);

    // Receive changes
    ChangeReceiver recv;
    t.sig_selectionChange.add(&recv, &ChangeReceiver::onSelectionChange);

    // Produce changes through proxy
    t.setCurrentLayer(4);

    // Wait for update
    while (recv.infos.empty()) {
        TS_ASSERT(disp.wait(1000));
    }

    TS_ASSERT_EQUALS(recv.infos.back().currentLayer, 4U);
}

/** Test signalisation of changes, internal.
    A: create session with some selections. Initiate change via proxy.
    E: change must be reflected to UI side. */
void
TestGameProxySelectionProxy::testSignalInternal()
{
    // Environment
    SessionThread h;
    prepare(h);
    SimpleRequestDispatcher disp;
    SelectionProxy t(h.gameSender(), disp);

    // Receive changes
    ChangeReceiver recv;
    t.sig_selectionChange.add(&recv, &ChangeReceiver::onSelectionChange);

    // Produce changes behind our back
    class Task : public util::Request<game::Session> {
     public:
        virtual void handle(game::Session& session)
            { session.getGame()->selections().setCurrentLayer(4, session.getGame()->currentTurn().universe()); }
    };
    h.gameSender().postNewRequest(new Task());

    // Wait for update
    while (recv.infos.empty()) {
        TS_ASSERT(disp.wait(1000));
    }

    TS_ASSERT_EQUALS(recv.infos.back().currentLayer, 4U);
}

/** Test clearLayer().
    A: create session with some selections. Call clearLayer().
    E: verify correct status can be read back. */
void
TestGameProxySelectionProxy::testClearLayer()
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    t.clearLayer(3);
    
    SelectionProxy::Info info;
    TS_ASSERT_THROWS_NOTHING(t.init(ind, info));
    TS_ASSERT_EQUALS(info.currentLayer, 0U);
    TS_ASSERT(!info.layers.empty());
    TS_ASSERT_EQUALS(info.layers[0].numPlanets, 1U);
    TS_ASSERT_EQUALS(info.layers[0].numShips, 2U);
    TS_ASSERT_EQUALS(info.layers[3].numPlanets, 0U);
    TS_ASSERT_EQUALS(info.layers[3].numShips, 0U);
}

/** Test clearAllLayers().
    A: create session with some selections. Call clearAllLayers().
    E: verify correct status can be read back. */
void
TestGameProxySelectionProxy::testClearAllLayers()
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    t.clearAllLayers();
    
    SelectionProxy::Info info;
    TS_ASSERT_THROWS_NOTHING(t.init(ind, info));
    TS_ASSERT_EQUALS(info.currentLayer, 0U);
    TS_ASSERT(!info.layers.empty());
    TS_ASSERT_EQUALS(info.layers[0].numPlanets, 0U);
    TS_ASSERT_EQUALS(info.layers[0].numShips, 0U);
    TS_ASSERT_EQUALS(info.layers[3].numPlanets, 0U);
    TS_ASSERT_EQUALS(info.layers[3].numShips, 0U);
}

/** Test invertLayer().
    A: create session with some selections. Call invertLayer().
    E: verify correct status can be read back. */
void
TestGameProxySelectionProxy::testInvertLayer()
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    t.invertLayer(0);
    
    SelectionProxy::Info info;
    TS_ASSERT_THROWS_NOTHING(t.init(ind, info));
    TS_ASSERT_EQUALS(info.currentLayer, 0U);
    TS_ASSERT(!info.layers.empty());
    TS_ASSERT_EQUALS(info.layers[0].numPlanets, 2U);
    TS_ASSERT_EQUALS(info.layers[0].numShips, 3U);
    TS_ASSERT_EQUALS(info.layers[3].numPlanets, 1U);
    TS_ASSERT_EQUALS(info.layers[3].numShips, 1U);
}

/** Test invertAllLayers().
    A: create session with some selections. Call invertAllLayers().
    E: verify correct status can be read back. */
void
TestGameProxySelectionProxy::testInvertAllLayers()
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    t.invertAllLayers();
    
    SelectionProxy::Info info;
    TS_ASSERT_THROWS_NOTHING(t.init(ind, info));
    TS_ASSERT_EQUALS(info.currentLayer, 0U);
    TS_ASSERT(!info.layers.empty());
    TS_ASSERT_EQUALS(info.layers[0].numPlanets, 2U);
    TS_ASSERT_EQUALS(info.layers[0].numShips, 3U);
    TS_ASSERT_EQUALS(info.layers[3].numPlanets, 2U);
    TS_ASSERT_EQUALS(info.layers[3].numShips, 4U);
}

/** Test executeExpression().
    A: create session with some selections. Call executeExpression() with a valid expression.
    E: verify correct status can be read back. */
void
TestGameProxySelectionProxy::testExecute()
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    String_t error;
    bool ok = t.executeExpression(ind, "current + d", 2, error);
    TS_ASSERT(ok);
    
    SelectionProxy::Info info;
    TS_ASSERT_THROWS_NOTHING(t.init(ind, info));
    TS_ASSERT_EQUALS(info.currentLayer, 0U);
    TS_ASSERT(!info.layers.empty());
    TS_ASSERT_EQUALS(info.layers[0].numPlanets, 1U);
    TS_ASSERT_EQUALS(info.layers[0].numShips, 2U);
    TS_ASSERT_EQUALS(info.layers[2].numPlanets, 2U);
    TS_ASSERT_EQUALS(info.layers[2].numShips, 3U);
    TS_ASSERT_EQUALS(info.layers[3].numPlanets, 1U);
    TS_ASSERT_EQUALS(info.layers[3].numShips, 1U);
}

/** Test executeExpression(), failure case.
    A: create session with some selections. Call executeExpression() with an invalid expression.
    E: error must be reported correctly. */
void
TestGameProxySelectionProxy::testExecuteFail()
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method
    {
        String_t error;
        bool ok = t.executeExpression(ind, "a+", 2, error);
        TS_ASSERT(!ok);
        TS_ASSERT(!error.empty());
    }

    // Alternative error path
    {
        String_t error;
        bool ok = t.executeExpression(ind, "a)", 2, error);
        TS_ASSERT(!ok);
        TS_ASSERT(!error.empty());
    }
}

