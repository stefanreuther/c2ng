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
        p.internalCheck(game::map::Configuration(), game::PlayerSet_t(), 15, tx, log);
        p.setPlayability(game::map::Object::NotPlayable);
        return p;
    }

    Ship& createShip(Universe& u, game::Id_t id)
    {
        Ship& s = *u.ships().create(id);
        s.addShipXYData(game::map::Point(1000, 1000+id), 3, 222, game::PlayerSet_t(1));
        s.internalCheck(game::PlayerSet_t(1), 15);
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

    struct CountReceiver {
        int n;

        CountReceiver()
            : n(0)
            { }
        void onNumObjectsInRange(int n)
            { this->n = n; }
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

    // Produce changes through proxy
    t.setCurrentLayer(4);

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

/** Test markList().
    A: create session with some objects. Call markList().
    E: objects must be marked correctly. */
void
TestGameProxySelectionProxy::testMarkList()
{
    using game::Reference;

    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Execute
    game::ref::List list;
    list.add(Reference(Reference::Ship, 13));
    list.add(Reference(Reference::Planet, 20));
    t.markList(0, list, true);

    // Verify
    h.sync();

    game::map::Universe& univ = h.session().getGame()->currentTurn().universe();
    TS_ASSERT_EQUALS(univ.ships().get(13)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(20)->isMarked(), true);
}

/** Test markObjectsInRange().
    A: create session with some objects. Call markObjectsInRange().
    E: verify correct result reported and object status. */
void
TestGameProxySelectionProxy::testMarkRange()
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);
    CountReceiver recv;
    t.sig_numObjectsInRange.add(&recv, &CountReceiver::onNumObjectsInRange);

    // Initial state has all objects at X=1000, Y=1000+id.
    // Planets: 10 (marked), 20, 30
    // Ships:   11, 12 (marked); 13, 14 (marked), 15
    game::map::Universe& univ = h.session().getGame()->currentTurn().universe();
    TS_ASSERT_EQUALS(univ.planets().get(10)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(20)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.planets().get(30)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(11)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(12)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(13)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(14)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(15)->isMarked(), false);

    // Mark range (1000,1015) - (1000,1030); this will mark the remaining two planets and one ship
    t.markObjectsInRange(game::map::Point(1000, 1015), game::map::Point(1000, 1030), true);
    h.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(univ.planets().get(10)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(20)->isMarked(), true); // changed
    TS_ASSERT_EQUALS(univ.planets().get(30)->isMarked(), true); // changed
    TS_ASSERT_EQUALS(univ.ships().get(11)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(12)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(13)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(14)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(15)->isMarked(), true);  // changed
    TS_ASSERT_EQUALS(recv.n, 3);

    // Mark range (1000,1019) - (1000,1021) without revert; this will not change anything
    t.markObjectsInRange(game::map::Point(1000, 1019), game::map::Point(1000, 1021), false);
    h.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(univ.planets().get(10)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(20)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(30)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(11)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(12)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(13)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(14)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(15)->isMarked(), true);
    TS_ASSERT_EQUALS(recv.n, 1);

    // Now with revert
    t.markObjectsInRange(game::map::Point(1000, 1019), game::map::Point(1000, 1021), true);
    h.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(univ.planets().get(10)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(20)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(30)->isMarked(), false); // reverted
    TS_ASSERT_EQUALS(univ.ships().get(11)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(12)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(13)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(14)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(15)->isMarked(), false); // reverted
    TS_ASSERT_EQUALS(recv.n, 1);

    // Revert everything
    t.revertCurrentLayer();
    h.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(univ.planets().get(10)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(20)->isMarked(), false); // also reverted
    TS_ASSERT_EQUALS(univ.planets().get(30)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(11)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(12)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(13)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(14)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(15)->isMarked(), false);
    TS_ASSERT_EQUALS(recv.n, 1); // no callback, value remains unchanged
}

/** Test markObjectsInRange(), wrapped-map case. Modified version of testMarkRange().
    A: create session with some objects. Call markObjectsInRange() with wrap.
    E: verify correct result reported and object status. */
void
TestGameProxySelectionProxy::testMarkRangeWrap()
{
    // Environment
    SessionThread h;
    prepare(h);
    h.session().getGame()->mapConfiguration().setConfiguration(game::map::Configuration::Wrapped, game::map::Point(2000, 2000), game::map::Point(2000, 2000));
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Initial state has all objects at X=1000, Y=1000+id.
    // Use range from X=[2900, 3100] to cover X=1000.
    // Use range from Y=[2900, 3011] to cover Y=[1000,1011]
    game::map::Universe& univ = h.session().getGame()->currentTurn().universe();

    t.markObjectsInRange(game::map::Point(3100, 2900), game::map::Point(2900, 3011), true);
    h.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(univ.planets().get(10)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(20)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.planets().get(30)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(11)->isMarked(), true); // changed
    TS_ASSERT_EQUALS(univ.ships().get(12)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(13)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.ships().get(14)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(15)->isMarked(), false);
}

