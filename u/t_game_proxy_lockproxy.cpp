/**
  *  \file u/t_game_proxy_lockproxy.cpp
  *  \brief Test for game::proxy::LockProxy
  */

#include <vector>
#include "game/proxy/lockproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "util/simplerequestdispatcher.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"
#include "game/game.hpp"
#include "game/test/root.hpp"

namespace {
    using afl::base::Ptr;
    using game::Game;
    using game::HostVersion;
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
}

/** Test empty universe.
    A: create empty session.
    E: call postQuery(). Must produce correct result (same as query). */
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
    t.postQuery(Point(1000, 1100), LockProxy::Flags_t());

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1100));
}

/** Test normal operation.
    A: create session with some objects.
    E: call postQuery(). Must produce correct result. */
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
    t.postQuery(Point(1200, 1120), LockProxy::Flags_t());

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1120));
}

/** Test debouncing.
    A: create session with some objects.
    E: call postQuery() multiple times. Must report only last result. */
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
    t.postQuery(Point(1200, 1120), LockProxy::Flags_t());
    t.postQuery(Point(1200, 1150), LockProxy::Flags_t());

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1150));
}


/** Test limitation to marked objects.
    A: create session with some objects; only one is marked.
    E: call postQuery(). Must produce correct result. */
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
    t.postQuery(Point(1200, 1120), LockProxy::Flags_t(LockProxy::MarkedOnly));

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1170));
}

/** Test limitation to range objects.
    A: create session with some objects.
    E: call setRangeLimit(), then postQuery(). Must produce correct result. */
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
    t.postQuery(Point(1200, 1150), LockProxy::Flags_t());

    // Wait for result
    while (recv.results.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(1000, 1140));
}

