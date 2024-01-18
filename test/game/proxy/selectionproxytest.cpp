/**
  *  \file test/game/proxy/selectionproxytest.cpp
  *  \brief Test for game::proxy::SelectionProxy
  */

#include "game/proxy/selectionproxy.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.proxy.SelectionProxy:empty", a)
{
    // Environment
    SessionThread h;
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call init()
    SelectionProxy::Info info;
    AFL_CHECK_SUCCEEDS(a("01. init"), t.init(ind, info));
    a.checkEqual("02. currentLayer", info.currentLayer, 0U);
    a.check("03. layers", info.layers.empty());

    // Call executeExpression
    // (we don't care whether this is reported as error or not, but it must not throw or hang)
    String_t error;
    AFL_CHECK_SUCCEEDS(a("11. executeExpression"), t.executeExpression(ind, "A", 1, error));
}

/** Test normal initialisation.
    A: create session with some selections.
    E: init() must report correct result. */
AFL_TEST("game.proxy.SelectionProxy:init", a)
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call init()
    SelectionProxy::Info info;
    AFL_CHECK_SUCCEEDS(a("01. init"), t.init(ind, info));
    a.checkEqual("02. currentLayer", info.currentLayer, 0U);
    a.check("03. size", info.layers.size() > 3U);
    a.checkEqual("04. numPlanets", info.layers[0].numPlanets, 1U);
    a.checkEqual("05. numShips",   info.layers[0].numShips, 2U);
    a.checkEqual("06. numPlanets", info.layers[3].numPlanets, 1U);
    a.checkEqual("07. numShips",   info.layers[3].numShips, 1U);
}

/** Test signalisation of changes, external.
    A: create session with some selections. Initiate change on game side.
    E: change must be reflected to UI side. */
AFL_TEST("game.proxy.SelectionProxy:signal:external", a)
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
        a.check("01. wait", disp.wait(1000));
    }

    a.checkEqual("11. currentLayer", recv.infos.back().currentLayer, 4U);
}

/** Test signalisation of changes, internal.
    A: create session with some selections. Initiate change via proxy.
    E: change must be reflected to UI side. */
AFL_TEST("game.proxy.SelectionProxy:signal:internal", a)
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
        a.check("01. wait", disp.wait(1000));
    }

    a.checkEqual("11. currentLayer", recv.infos.back().currentLayer, 4U);
}

/** Test clearLayer().
    A: create session with some selections. Call clearLayer().
    E: verify correct status can be read back. */
AFL_TEST("game.proxy.SelectionProxy:clearLayer", a)
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    t.clearLayer(3);

    SelectionProxy::Info info;
    AFL_CHECK_SUCCEEDS(a("01. init"), t.init(ind, info));
    a.checkEqual("02. currentLayer", info.currentLayer, 0U);
    a.check("03. layers", !info.layers.empty());
    a.checkEqual("04. numPlanets", info.layers[0].numPlanets, 1U);
    a.checkEqual("05. numShips", info.layers[0].numShips, 2U);
    a.checkEqual("06. numPlanets", info.layers[3].numPlanets, 0U);
    a.checkEqual("07. numShips", info.layers[3].numShips, 0U);
}

/** Test clearAllLayers().
    A: create session with some selections. Call clearAllLayers().
    E: verify correct status can be read back. */
AFL_TEST("game.proxy.SelectionProxy:clearAllLayers", a)
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    t.clearAllLayers();

    SelectionProxy::Info info;
    AFL_CHECK_SUCCEEDS(a("01. init"), t.init(ind, info));
    a.checkEqual("02. currentLayer", info.currentLayer, 0U);
    a.check("03. layers", !info.layers.empty());
    a.checkEqual("04. numPlanets", info.layers[0].numPlanets, 0U);
    a.checkEqual("05. numShips", info.layers[0].numShips, 0U);
    a.checkEqual("06. numPlanets", info.layers[3].numPlanets, 0U);
    a.checkEqual("07. numShips", info.layers[3].numShips, 0U);
}

/** Test invertLayer().
    A: create session with some selections. Call invertLayer().
    E: verify correct status can be read back. */
AFL_TEST("game.proxy.SelectionProxy:invertLayer", a)
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    t.invertLayer(0);

    SelectionProxy::Info info;
    AFL_CHECK_SUCCEEDS(a("01. init"), t.init(ind, info));
    a.checkEqual("02. currentLayer", info.currentLayer, 0U);
    a.check("03. layers", !info.layers.empty());
    a.checkEqual("04. numPlanets", info.layers[0].numPlanets, 2U);
    a.checkEqual("05. numShips", info.layers[0].numShips, 3U);
    a.checkEqual("06. numPlanets", info.layers[3].numPlanets, 1U);
    a.checkEqual("07. numShips", info.layers[3].numShips, 1U);
}

/** Test invertAllLayers().
    A: create session with some selections. Call invertAllLayers().
    E: verify correct status can be read back. */
AFL_TEST("game.proxy.SelectionProxy:invertAllLayers", a)
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    t.invertAllLayers();

    SelectionProxy::Info info;
    AFL_CHECK_SUCCEEDS(a("01. init"), t.init(ind, info));
    a.checkEqual("02. currentLayer", info.currentLayer, 0U);
    a.check("03. layers", !info.layers.empty());
    a.checkEqual("04. numPlanets", info.layers[0].numPlanets, 2U);
    a.checkEqual("05. numShips", info.layers[0].numShips, 3U);
    a.checkEqual("06. numPlanets", info.layers[3].numPlanets, 2U);
    a.checkEqual("07. numShips", info.layers[3].numShips, 4U);
}

/** Test executeExpression().
    A: create session with some selections. Call executeExpression() with a valid expression.
    E: verify correct status can be read back. */
AFL_TEST("game.proxy.SelectionProxy:executeExpression", a)
{
    // Environment
    SessionThread h;
    prepare(h);
    WaitIndicator ind;
    SelectionProxy t(h.gameSender(), ind);

    // Call method-under-test, then read back result.
    String_t error;
    bool ok = t.executeExpression(ind, "current + d", 2, error);
    a.check("01. executeExpression", ok);

    SelectionProxy::Info info;
    AFL_CHECK_SUCCEEDS(a("11. init"), t.init(ind, info));
    a.checkEqual("12. currentLayer", info.currentLayer, 0U);
    a.check("13. layers", !info.layers.empty());
    a.checkEqual("14. numPlanets", info.layers[0].numPlanets, 1U);
    a.checkEqual("15. numShips", info.layers[0].numShips, 2U);
    a.checkEqual("16. numPlanets", info.layers[2].numPlanets, 2U);
    a.checkEqual("17. numShips", info.layers[2].numShips, 3U);
    a.checkEqual("18. numPlanets", info.layers[3].numPlanets, 1U);
    a.checkEqual("19. numShips", info.layers[3].numShips, 1U);
}

/** Test executeExpression(), failure case.
    A: create session with some selections. Call executeExpression() with an invalid expression.
    E: error must be reported correctly. */
AFL_TEST("game.proxy.SelectionProxy:executeExpression:error", a)
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
        a.check("01. executeExpression", !ok);
        a.check("02. empty", !error.empty());
    }

    // Alternative error path
    {
        String_t error;
        bool ok = t.executeExpression(ind, "a)", 2, error);
        a.check("11. executeExpression", !ok);
        a.check("12. empty", !error.empty());
    }
}

/** Test markList().
    A: create session with some objects. Call markList().
    E: objects must be marked correctly. */
AFL_TEST("game.proxy.SelectionProxy:markList", a)
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
    a.checkEqual("01. isMarked", univ.ships().get(13)->isMarked(), true);
    a.checkEqual("02. isMarked", univ.planets().get(20)->isMarked(), true);
}

/** Test markObjectsInRange().
    A: create session with some objects. Call markObjectsInRange().
    E: verify correct result reported and object status. */
AFL_TEST("game.proxy.SelectionProxy:markObjectsInRange", a)
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
    a.checkEqual("01. isMarked", univ.planets().get(10)->isMarked(), true);
    a.checkEqual("02. isMarked", univ.planets().get(20)->isMarked(), false);
    a.checkEqual("03. isMarked", univ.planets().get(30)->isMarked(), false);
    a.checkEqual("04. isMarked", univ.ships().get(11)->isMarked(), false);
    a.checkEqual("05. isMarked", univ.ships().get(12)->isMarked(), true);
    a.checkEqual("06. isMarked", univ.ships().get(13)->isMarked(), false);
    a.checkEqual("07. isMarked", univ.ships().get(14)->isMarked(), true);
    a.checkEqual("08. isMarked", univ.ships().get(15)->isMarked(), false);

    // Mark range (1000,1015) - (1000,1030); this will mark the remaining two planets and one ship
    t.markObjectsInRange(game::map::Point(1000, 1015), game::map::Point(1000, 1030), true);
    h.sync();
    ind.processQueue();

    a.checkEqual("11. isMarked", univ.planets().get(10)->isMarked(), true);
    a.checkEqual("12. isMarked", univ.planets().get(20)->isMarked(), true); // changed
    a.checkEqual("13. isMarked", univ.planets().get(30)->isMarked(), true); // changed
    a.checkEqual("14. isMarked", univ.ships().get(11)->isMarked(), false);
    a.checkEqual("15. isMarked", univ.ships().get(12)->isMarked(), true);
    a.checkEqual("16. isMarked", univ.ships().get(13)->isMarked(), false);
    a.checkEqual("17. isMarked", univ.ships().get(14)->isMarked(), true);
    a.checkEqual("18. isMarked", univ.ships().get(15)->isMarked(), true);  // changed
    a.checkEqual("19. n", recv.n, 3);

    // Mark range (1000,1019) - (1000,1021) without revert; this will not change anything
    t.markObjectsInRange(game::map::Point(1000, 1019), game::map::Point(1000, 1021), false);
    h.sync();
    ind.processQueue();

    a.checkEqual("21. isMarked", univ.planets().get(10)->isMarked(), true);
    a.checkEqual("22. isMarked", univ.planets().get(20)->isMarked(), true);
    a.checkEqual("23. isMarked", univ.planets().get(30)->isMarked(), true);
    a.checkEqual("24. isMarked", univ.ships().get(11)->isMarked(), false);
    a.checkEqual("25. isMarked", univ.ships().get(12)->isMarked(), true);
    a.checkEqual("26. isMarked", univ.ships().get(13)->isMarked(), false);
    a.checkEqual("27. isMarked", univ.ships().get(14)->isMarked(), true);
    a.checkEqual("28. isMarked", univ.ships().get(15)->isMarked(), true);
    a.checkEqual("29. n", recv.n, 1);

    // Now with revert
    t.markObjectsInRange(game::map::Point(1000, 1019), game::map::Point(1000, 1021), true);
    h.sync();
    ind.processQueue();

    a.checkEqual("31. isMarked", univ.planets().get(10)->isMarked(), true);
    a.checkEqual("32. isMarked", univ.planets().get(20)->isMarked(), true);
    a.checkEqual("33. isMarked", univ.planets().get(30)->isMarked(), false); // reverted
    a.checkEqual("34. isMarked", univ.ships().get(11)->isMarked(), false);
    a.checkEqual("35. isMarked", univ.ships().get(12)->isMarked(), true);
    a.checkEqual("36. isMarked", univ.ships().get(13)->isMarked(), false);
    a.checkEqual("37. isMarked", univ.ships().get(14)->isMarked(), true);
    a.checkEqual("38. isMarked", univ.ships().get(15)->isMarked(), false); // reverted
    a.checkEqual("39. n", recv.n, 1);

    // Revert everything
    t.revertCurrentLayer();
    h.sync();
    ind.processQueue();

    a.checkEqual("41. isMarked", univ.planets().get(10)->isMarked(), true);
    a.checkEqual("42. isMarked", univ.planets().get(20)->isMarked(), false); // also reverted
    a.checkEqual("43. isMarked", univ.planets().get(30)->isMarked(), false);
    a.checkEqual("44. isMarked", univ.ships().get(11)->isMarked(), false);
    a.checkEqual("45. isMarked", univ.ships().get(12)->isMarked(), true);
    a.checkEqual("46. isMarked", univ.ships().get(13)->isMarked(), false);
    a.checkEqual("47. isMarked", univ.ships().get(14)->isMarked(), true);
    a.checkEqual("48. isMarked", univ.ships().get(15)->isMarked(), false);
    a.checkEqual("49. n", recv.n, 1); // no callback, value remains unchanged
}

/** Test markObjectsInRange(), wrapped-map case. Modified version of testMarkRange().
    A: create session with some objects. Call markObjectsInRange() with wrap.
    E: verify correct result reported and object status. */
AFL_TEST("game.proxy.SelectionProxy:markObjectsInRange:wrap", a)
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

    a.checkEqual("01. isMarked", univ.planets().get(10)->isMarked(), true);
    a.checkEqual("02. isMarked", univ.planets().get(20)->isMarked(), false);
    a.checkEqual("03. isMarked", univ.planets().get(30)->isMarked(), false);
    a.checkEqual("04. isMarked", univ.ships().get(11)->isMarked(), true); // changed
    a.checkEqual("05. isMarked", univ.ships().get(12)->isMarked(), true);
    a.checkEqual("06. isMarked", univ.ships().get(13)->isMarked(), false);
    a.checkEqual("07. isMarked", univ.ships().get(14)->isMarked(), true);
    a.checkEqual("08. isMarked", univ.ships().get(15)->isMarked(), false);
}
