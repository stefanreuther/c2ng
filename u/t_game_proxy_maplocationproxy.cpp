/**
  *  \file u/t_game_proxy_maplocationproxy.cpp
  *  \brief Test for game::proxy::MapLocationProxy
  */

#include "game/proxy/maplocationproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/turn.hpp"
#include "util/simplerequestdispatcher.hpp"

using game::Reference;
using game::map::Configuration;
using game::map::Point;
using game::proxy::MapLocationProxy;
using game::test::SessionThread;
using util::SimpleRequestDispatcher;

namespace {
    struct ResultReceiver {
        Reference ref;
        Point point;
        Configuration config;
        bool ok;

        void onLocationResult(Reference ref, Point point, Configuration config)
            {
                this->ref = ref;
                this->point = point;
                this->config = config;
                this->ok = true;
            }

        ResultReceiver()
            : ref(), point(), config(), ok(false)
            { }
    };

    struct PositionReceiver {
        std::vector<Point> positions;

        void onPositionChange(Point pt)
            { positions.push_back(pt); }
    };

    void prepare(SessionThread& s)
    {
        s.session().setRoot(new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,2,0))));
        s.session().setGame(new game::Game());
    }

    void addShip(SessionThread& s, int id, Point pos)
    {
        game::map::Ship* sh = s.session().getGame()->currentTurn().universe().ships().create(id);
        sh->addShipXYData(pos, 1, 100, game::PlayerSet_t(1));
        sh->internalCheck();
    }
}

/** Test empty session.
    A: create empty session. Call postQueryLocation().
    E: callback must be generated */
void
TestGameProxyMapLocationProxy::testEmpty()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread s;
    SimpleRequestDispatcher disp;
    MapLocationProxy testee(s.gameSender(), disp);

    // Post query
    ResultReceiver recv;
    testee.sig_locationResult.add(&recv, &ResultReceiver::onLocationResult);
    testee.postQueryLocation();
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }
}

/** Test point access.
    A: create session with a universe. Call setPosition(Point).
    E: sig_positionChange callback created. postQueryLocation() answered correctly. */
void
TestGameProxyMapLocationProxy::testPoint()
{
    const Point POS(1300, 1300);

    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread s;
    prepare(s);
    SimpleRequestDispatcher disp;
    MapLocationProxy testee(s.gameSender(), disp);

    // Callbacks
    ResultReceiver recv;
    testee.sig_locationResult.add(&recv, &ResultReceiver::onLocationResult);

    PositionReceiver pos;
    testee.sig_positionChange.add(&pos, &PositionReceiver::onPositionChange);

    // Set position
    testee.setPosition(POS);
    while (pos.positions.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(pos.positions[0], POS);

    // Post query
    testee.postQueryLocation();
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.point, POS);
    TS_ASSERT_EQUALS(recv.ref, Reference());
}

/** Test reference access.
    A: create session with a universe. Call setPosition(Reference).
    E: sig_positionChange callback created. postQueryLocation() answered correctly. */
void
TestGameProxyMapLocationProxy::testReference()
{
    const Point POS(1492, 1902);
    const int ID = 99;

    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread s;
    prepare(s);
    addShip(s, ID, POS);
    SimpleRequestDispatcher disp;
    MapLocationProxy testee(s.gameSender(), disp);

    // Callbacks
    ResultReceiver recv;
    testee.sig_locationResult.add(&recv, &ResultReceiver::onLocationResult);

    PositionReceiver pos;
    testee.sig_positionChange.add(&pos, &PositionReceiver::onPositionChange);

    // Set position
    testee.setPosition(Reference(Reference::Ship, ID));
    while (pos.positions.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(pos.positions[0], POS);

    // Post query
    testee.postQueryLocation();
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.point, POS);
    TS_ASSERT_EQUALS(recv.ref, Reference(Reference::Ship, ID));
}
