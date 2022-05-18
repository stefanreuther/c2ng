/**
  *  \file u/t_game_proxy_lockproxy.cpp
  *  \brief Test for game::proxy::LockProxy
  */

#include <vector>
#include "game/proxy/lockproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "util/simplerequestdispatcher.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"
#include "game/game.hpp"
#include "game/test/root.hpp"

namespace {
    using afl::base::Ptr;
    using game::Game;
    using game::HostVersion;
    using game::map::Planet;
    using game::map::Point;
    using game::map::Ship;
    using game::map::Universe;
    using game::proxy::LockProxy;
    using game::test::Root;
    using game::test::SessionThread;
    using util::SimpleRequestDispatcher;

    struct ResultReceiver {
        std::vector<Point> results;

        void onResult(Point pt)
            { results.push_back(pt); }
    };

    struct UnitNameResultReceiver {
        std::vector<std::pair<Point, String_t> > results;

        void onResult(Point pt, String_t name)
            { results.push_back(std::make_pair(pt, name)); }
    };

    void prepare(SessionThread& h)
    {
        Ptr<Root> r = new game::test::Root(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)));
        h.session().setRoot(r);

        // Ships at positions (1000,1110), (1000,1120), ... (1000,1200)
        Ptr<Game> g = new Game();
        Universe& univ = g->currentTurn().universe();
        for (int i = 1; i < 10; ++i) {
            Ship* p = univ.ships().create(i);
            TS_ASSERT(p);
            p->addShipXYData(Point(1000, 1100 + 10*i), 1, 100, game::PlayerSet_t(1));
            p->internalCheck();
            if (i == 7) {
                p->setIsMarked(true);
            }
        }
        h.session().setGame(g);
    }

    void addPlanet(SessionThread& h)
    {
        Ptr<Game> g = h.session().getGame();
        Universe& univ = g->currentTurn().universe();

        Planet& p = *univ.planets().create(333);
        p.setPosition(Point(2000, 2000));
        p.internalCheck(g->mapConfiguration(), h.session().translator(), h.session().log());
    }
}

/** Test empty universe, requestPosition().
    A: create empty session.
    E: call requestPosition(). Must produce correct result (same as query). */
void
TestGameProxyLockProxy::testEmpty()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    ResultReceiver recv;
    t.sig_result.add(&recv, &ResultReceiver::onResult);
    t.requestPosition(Point(1000, 1100), LockProxy::Flags_t());

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1100));
}

/** Test empty universe, requestUnitNames().
    A: create empty session.
    E: call requestUnitNames(). Must produce correct result (same as query). */
void
TestGameProxyLockProxy::testEmptyName()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    UnitNameResultReceiver recv;
    t.sig_unitNameResult.add(&recv, &UnitNameResultReceiver::onResult);
    t.requestUnitNames(Point(1000, 1100));

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0].first, Point(1000, 1100));
    TS_ASSERT_EQUALS(recv.results[0].second, "");
}

/** Test normal operation, requestPosition().
    A: create session with some objects.
    E: call requestPosition(). Must produce correct result. */
void
TestGameProxyLockProxy::testNormal()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    prepare(h);
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    ResultReceiver recv;
    t.sig_result.add(&recv, &ResultReceiver::onResult);
    t.requestPosition(Point(1200, 1120), LockProxy::Flags_t());

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1120));
}

/** Test normal operation, requestUnitNames().
    A: create session with some objects.
    E: call requestUnitNames(). Must produce correct result. */
void
TestGameProxyLockProxy::testNormalName()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    prepare(h);
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    UnitNameResultReceiver recv;
    t.sig_unitNameResult.add(&recv, &UnitNameResultReceiver::onResult);
    t.requestUnitNames(Point(1200, 1120));

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0].first, Point(1000, 1120));
    TS_ASSERT_EQUALS(recv.results[0].second, "1 Player 1 ship");
}

/** Test debouncing, requestPosition().
    A: create session with some objects.
    E: call requestPosition() multiple times. Must report only last result. */
void
TestGameProxyLockProxy::testRepeat()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    prepare(h);
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    ResultReceiver recv;
    t.sig_result.add(&recv, &ResultReceiver::onResult);
    t.requestPosition(Point(1200, 1120), LockProxy::Flags_t());
    t.requestPosition(Point(1200, 1150), LockProxy::Flags_t());

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1150));
}

/** Test debouncing, requestUnitNames().
    A: create session with some objects.
    E: call requestUnitNames() multiple times. Must report only last result. */
void
TestGameProxyLockProxy::testRepeatName()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    prepare(h);
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    UnitNameResultReceiver recv;
    t.sig_unitNameResult.add(&recv, &UnitNameResultReceiver::onResult);
    t.requestUnitNames(Point(1200, 1120));
    t.requestUnitNames(Point(1200, 1150));

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0].first, Point(1000, 1150));
    TS_ASSERT_EQUALS(recv.results[0].second, "1 Player 1 ship");
}

/** Test limitation to marked objects.
    A: create session with some objects; only one is marked.
    E: call requestPosition(). Must produce correct result. */
void
TestGameProxyLockProxy::testMarked()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    prepare(h);
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    ResultReceiver recv;
    t.sig_result.add(&recv, &ResultReceiver::onResult);
    t.requestPosition(Point(1200, 1120), LockProxy::Flags_t(LockProxy::MarkedOnly));

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1170));
}

/** Test limitation to range objects, requestPosition().
    A: create session with some objects.
    E: call setRangeLimit(), then requestPosition(). Must produce correct result. */
void
TestGameProxyLockProxy::testRange()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    prepare(h);
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    ResultReceiver recv;
    t.sig_result.add(&recv, &ResultReceiver::onResult);
    t.setRangeLimit(Point(1000, 1000), Point(1200, 1140));
    t.requestPosition(Point(1200, 1150), LockProxy::Flags_t());

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1140));
}

/** Test limitation to range objects, requestUnitNames().
    A: create session with some objects.
    E: call setRangeLimit(), then requestUnitNames(). Must produce correct result. */
void
TestGameProxyLockProxy::testRangeName()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    prepare(h);
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    UnitNameResultReceiver recv;
    t.sig_unitNameResult.add(&recv, &UnitNameResultReceiver::onResult);
    t.setRangeLimit(Point(1000, 1000), Point(1200, 1140));
    t.requestUnitNames(Point(1200, 1150));

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0].first, Point(1000, 1140));
    TS_ASSERT_EQUALS(recv.results[0].second, "1 Player 1 ship");
}

/** Set setOrigin.
    A: create session with some objects including a planet.
    E: call setOrigin(); then call requestPosition(). Must produce correct result. */
void
TestGameProxyLockProxy::testSetOrigin()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread h;
    prepare(h);        // Ships at positions (1000,1110), (1000,1120), ... (1000,1200)
    addPlanet(h);      // Planet at position 2000,2000
    SimpleRequestDispatcher disp;
    LockProxy t(h.gameSender(), disp);

    // Testee
    ResultReceiver recv;
    t.sig_result.add(&recv, &ResultReceiver::onResult);
    t.setOrigin(Point(2100, 2000), false);
    t.requestPosition(Point(2010, 2010), LockProxy::Flags_t() + LockProxy::ToggleOptimizeWarp + LockProxy::Left);

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(2003, 2000));
}

