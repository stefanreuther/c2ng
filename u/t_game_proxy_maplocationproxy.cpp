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
#include "game/test/waitindicator.hpp"
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
        bool configOk;

        void onLocationResult(Reference ref, Point point, Configuration config)
            {
                this->ref = ref;
                this->point = point;
                this->config = config;
                this->ok = true;
            }

        void onConfigChange(Configuration config)
            {
                this->config = config;
                this->configOk = true;
            }

        ResultReceiver()
            : ref(), point(), config(), ok(false), configOk(false)
            { }
    };

    struct PositionReceiver {
        std::vector<Point> positions;

        void onPositionChange(Point pt)
            { positions.push_back(pt); }
    };

    struct BrowseReceiver {
        Reference ref;
        Point point;
        bool ok;

        void onBrowseResult(Reference ref, Point pt)
            {
                this->ref = ref;
                this->point = pt;
                this->ok = true;
            }

        BrowseReceiver()
            : ref(), point(), ok(false)
            { }
    };

    void prepare(SessionThread& s)
    {
        s.session().setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,2,0))).asPtr());
        s.session().setGame(new game::Game());
    }

    game::map::Ship& addShip(SessionThread& s, int id, Point pos)
    {
        game::map::Ship* sh = s.session().getGame()->currentTurn().universe().ships().create(id);
        sh->addShipXYData(pos, 1, 100, game::PlayerSet_t(2));
        sh->internalCheck(game::PlayerSet_t(2), 15);
        return *sh;
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

/** Test browsing.
    A: create session with multiple ships. Call setPosition(Reference). Call browse().
    E: sig_browseResult callback created. postQueryLocation() answered correctly */
void
TestGameProxyMapLocationProxy::testBrowse()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread s;
    prepare(s);
    for (int i = 1; i < 10; ++i) {
        addShip(s, i, Point(1000, 1000+i));
    }
    SimpleRequestDispatcher disp;
    MapLocationProxy testee(s.gameSender(), disp);

    // Callbacks
    ResultReceiver recv;
    testee.sig_locationResult.add(&recv, &ResultReceiver::onLocationResult);

    PositionReceiver pos;
    testee.sig_positionChange.add(&pos, &PositionReceiver::onPositionChange);

    BrowseReceiver bro;
    testee.sig_browseResult.add(&bro, &BrowseReceiver::onBrowseResult);

    // Set position
    testee.setPosition(Reference(Reference::Ship, 3));
    while (pos.positions.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(pos.positions[0], Point(1000, 1003));
    TS_ASSERT_EQUALS(bro.ok, false);

    // Browse forward
    pos.positions.clear();
    testee.browse(game::map::Location::BrowseFlags_t(game::map::Location::Backwards));
    while (pos.positions.empty() || !bro.ok) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(pos.positions[0], Point(1000, 1002));
    TS_ASSERT_EQUALS(bro.ok, true);
    TS_ASSERT_EQUALS(bro.point, Point(1000, 1002));
    TS_ASSERT_EQUALS(bro.ref, Reference(Reference::Ship, 2));

    // Post query
    testee.postQueryLocation();
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.point, Point(1000, 1002));
    TS_ASSERT_EQUALS(recv.ref, Reference(Reference::Ship, 2));
}

/** Test configuration change.
    A: create session. Register sig_configChange callback. Modify configuration.
    E: sig_configChange callback generated */
void
TestGameProxyMapLocationProxy::testConfigChange()
{
    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread s;
    prepare(s);
    s.session().getRoot()->hostConfiguration()[game::config::HostConfiguration::AllowWraparoundMap].set(0);
    SimpleRequestDispatcher disp;
    MapLocationProxy testee(s.gameSender(), disp);

    // Set up receiver
    ResultReceiver recv;
    testee.sig_locationResult.add(&recv, &ResultReceiver::onLocationResult);
    testee.sig_configChange.add(&recv, &ResultReceiver::onConfigChange);
    testee.postQueryLocation();
    while (!recv.ok) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT(!recv.configOk); // no config callback yet
    TS_ASSERT_EQUALS(recv.config.getMode(), game::map::Configuration::Flat);

    // Modify configuration
    class ModTask : public util::Request<game::Session> {
     public:
        virtual void handle(game::Session& s)
            {
                s.getRoot()->hostConfiguration()[game::config::HostConfiguration::AllowWraparoundMap].set(1);
                s.notifyListeners();
            }
    };
    s.gameSender().postNewRequest(new ModTask());
    while (!recv.configOk) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.config.getMode(), game::map::Configuration::Wrapped);
}

/** Test getOtherPosition().
    A: create session with a ship with waypoint. Call getOtherPosition().
    E: correct value returned */
void
TestGameProxyMapLocationProxy::testGetOtherPosition()
{
    const game::Id_t ID = 100;
    const Point POS(1200, 1300);
    const Point WP(1400, 1700);

    // Environment
    CxxTest::setAbortTestOnFail(true);
    SessionThread s;
    game::test::WaitIndicator ind;
    prepare(s);
    addShip(s, ID, POS).setWaypoint(WP);

    // Testee
    MapLocationProxy testee(s.gameSender(), ind);
    testee.setPosition(POS);

    // Verify
    // - Failure case
    TS_ASSERT_EQUALS(testee.getOtherPosition(ind, 0).isValid(), false);

    // - Success case
    Point result;
    TS_ASSERT_EQUALS(testee.getOtherPosition(ind, ID).get(result), true);
    TS_ASSERT_EQUALS(result, WP);
}

