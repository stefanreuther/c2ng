/**
  *  \file u/t_game_proxy_minefieldproxy.cpp
  *  \brief Test for game::proxy::MinefieldProxy
  */

#include "game/proxy/minefieldproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/map/minefieldtype.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using game::map::Minefield;
using game::map::Point;
using game::proxy::MinefieldProxy;
using game::test::SessionThread;
using game::test::WaitIndicator;

namespace {
    const int TURN_NR = 50;

    const int ME = 1;
    const int ALLY = 2;
    const int THEM = 3;

    void prepare(SessionThread& h)
    {
        // Game with TurnScoreList, TeamSettings
        afl::base::Ptr<game::Game> g = new game::Game();
        g->currentTurn().setTurnNumber(TURN_NR);
        g->teamSettings().setViewpointPlayer(ME);
        h.session().setGame(g);

        // Root with PlayerList, HostVersion, Configuration
        afl::base::Ptr<game::Root> r = new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
        r->playerList().create(1)->setName(game::Player::ShortName, "The Feds");
        r->playerList().create(2)->setName(game::Player::ShortName, "The Lizards");
        r->playerList().create(3)->setName(game::Player::ShortName, "The Birds");
        r->playerList().create(1)->setName(game::Player::AdjectiveName, "Fed");
        r->playerList().create(2)->setName(game::Player::AdjectiveName, "Lizard");
        r->playerList().create(3)->setName(game::Player::AdjectiveName, "Bird");
        h.session().setRoot(r);

        // Ship list
        afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
        game::test::initStandardBeams(*sl);
        h.session().setShipList(sl);
    }

    void addMinefield(SessionThread& h, game::Id_t id, int owner, bool isWeb, Point pos, int32_t units, int turn)
    {
        h.session().getGame()->currentTurn().universe().minefields().create(id)
            ->addReport(pos, owner, isWeb ? Minefield::IsWeb : Minefield::IsMine, Minefield::UnitsKnown, units, turn, Minefield::MinefieldScanned);
    }

    void addPlanet(SessionThread& h, game::Id_t pid, int owner, Point pos, String_t name)
    {
        game::map::Planet* pl = h.session().getGame()->currentTurn().universe().planets().create(pid);

        pl->setName(name);
        pl->setPosition(pos);

        if (owner != 0) {
            game::map::PlanetData d;
            d.owner = owner;
            d.friendlyCode = "abc";
            d.colonistClans = 999;
            pl->addCurrentPlanetData(d, game::PlayerSet_t(owner));
        }
    }

    void postprocessUniverse(SessionThread& h)
    {
        game::Root* r = h.session().getRoot().get();
        h.session().getGame()->currentTurn().universe()
            .postprocess(game::PlayerSet_t() + ME,      // Playing
                         game::PlayerSet_t() + ME + ALLY,  // Available
                         game::map::Object::Playable,
                         h.session().getGame()->mapConfiguration(),
                         r->hostVersion(),
                         r->hostConfiguration(),
                         TURN_NR,
                         *h.session().getShipList(),
                         h.session().translator(),
                         h.session().log());

    }

    template<typename T>
    class Receiver {
     public:
        void onUpdate(const T& value)
            { m_value = value; }
        const T& get() const
            { return m_value; }
     private:
        T m_value;
    };
    typedef Receiver<MinefieldProxy::MinefieldInfo> MinefieldInfoReceiver_t;
    typedef Receiver<MinefieldProxy::PassageInfo> PassageInfoReceiver_t;
}

/** Test behaviour on empty session.
    A: create empty session. Create MinefieldProxy.
    E: default data reported */
void
TestGameProxyMinefieldProxy::testEmpty()
{
    WaitIndicator ind;
    SessionThread thread;
    MinefieldProxy t(ind, thread.gameSender());

    MinefieldProxy::SweepInfo info;
    t.getSweepInfo(ind, info);

    TS_ASSERT_EQUALS(info.units, 0);
    TS_ASSERT_EQUALS(info.isWeb, false);
    TS_ASSERT_EQUALS(info.weapons.size(), 0U);
}

/** Test behaviour on session with no minefields.
    A: create empty session. Create MinefieldProxy.
    E: must report minefield 0. */
void
TestGameProxyMinefieldProxy::testNoMine()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);                   // create game, but no minefield
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    // Connect listeners
    MinefieldInfoReceiver_t miReceiver;
    game::test::Counter miCounter;
    t.sig_minefieldChange.add(&miReceiver, &MinefieldInfoReceiver_t::onUpdate);
    t.sig_minefieldChange.add(&miCounter, &game::test::Counter::increment);

    // Receive initial request
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT(miCounter.get() > 0);
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 0);
}

/** Test normal behaviour.
    A: create session. Add a minefield. Create MinefieldProxy.
    E: correct data reported */
void
TestGameProxyMinefieldProxy::testNormal()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, ME, false, Point(1000, 2000), 20000, TURN_NR);
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    // Connect listeners
    MinefieldInfoReceiver_t miReceiver;
    t.sig_minefieldChange.add(&miReceiver, &MinefieldInfoReceiver_t::onUpdate);

    PassageInfoReceiver_t piReceiver;
    t.sig_passageChange.add(&piReceiver, &PassageInfoReceiver_t::onUpdate);

    // Receive initial request
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 200);
    TS_ASSERT_EQUALS(miReceiver.get().controllingPlanetId, 0);     // we don't have any planet
    TS_ASSERT_EQUALS(miReceiver.get().center.getX(), 1000);
    TS_ASSERT_EQUALS(miReceiver.get().center.getY(), 2000);
    TS_ASSERT_EQUALS(miReceiver.get().radius, 141);                // sqrt(20000)
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::Owner], "The Feds");
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::Radius], "141 ly radius");
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::Units], "20,000 units");
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::AfterDecay], "19,000 units (137 ly)");
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::LastInfo], "current turn");
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::ControlPlanet], "unknown");
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::ControlPlayer], "");

    TS_ASSERT_EQUALS(piReceiver.get().distance, 141);
    TS_ASSERT_DELTA(piReceiver.get().normalPassageRate, 0.24, 0.01);
    TS_ASSERT_DELTA(piReceiver.get().cloakedPassageRate, 0.49, 0.01);
}

/** Test planet association, own planet.
    A: create session with minefield owned by viewpoint player and a couple of planets. Create MinefieldProxy.
    E: correct planet reported */
void
TestGameProxyMinefieldProxy::testPlanetOwn()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, ME, false, Point(1000, 2000), 20000, TURN_NR);
    addPlanet(thread, 50, ME,   Point(1010, 2010), "Fifty");
    addPlanet(thread, 60, 0,    Point(1005, 2006), "Sixty");
    addPlanet(thread, 70, ALLY, Point(1005, 2005), "Seventy");
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    MinefieldInfoReceiver_t miReceiver;
    t.sig_minefieldChange.add(&miReceiver, &MinefieldInfoReceiver_t::onUpdate);

    thread.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(miReceiver.get().controllingPlanetId, 50);
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::ControlPlanet], "Fifty");
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::ControlPlayer], "our planet");
}

/** Test planet association, seen planet.
    A: create session with minefield owned by player with full data and a couple of planets. Create MinefieldProxy.
    E: correct planet reported */
void
TestGameProxyMinefieldProxy::testPlanetSeen()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, ALLY, false, Point(1000, 2000), 20000, TURN_NR);
    addPlanet(thread, 50, ME,   Point(1010, 2010), "Fifty");
    addPlanet(thread, 60, 0,    Point(1005, 2004), "Sixty");
    addPlanet(thread, 70, ALLY, Point(1005, 2005), "Seventy");
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    MinefieldInfoReceiver_t miReceiver;
    t.sig_minefieldChange.add(&miReceiver, &MinefieldInfoReceiver_t::onUpdate);

    thread.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(miReceiver.get().controllingPlanetId, 70);
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::ControlPlanet], "Seventy");
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::ControlPlayer], "The Lizards");
}

/** Test planet association, other planet.
    A: create session with minefield owned by foreign player. Create MinefieldProxy.
    E: correct planet reported */
void
TestGameProxyMinefieldProxy::testPlanetOther()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, THEM, false, Point(1000, 2000), 20000, TURN_NR);
    addPlanet(thread, 50, ME,   Point(1010, 2010), "Fifty");
    addPlanet(thread, 60, 0,    Point(1005, 2006), "Sixty");
    addPlanet(thread, 70, ALLY, Point(1005, 2005), "Seventy");
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    MinefieldInfoReceiver_t miReceiver;
    t.sig_minefieldChange.add(&miReceiver, &MinefieldInfoReceiver_t::onUpdate);

    thread.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(miReceiver.get().controllingPlanetId, 60);
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::ControlPlanet], "Sixty");
    TS_ASSERT_EQUALS(miReceiver.get().text[MinefieldProxy::ControlPlayer], "a planet with unknown owner");
}

/** Test passage rate configuration.
    A: create session with minefield. Create MinefieldProxy. Call setPassageDistance.
    E: correct updates reported */
void
TestGameProxyMinefieldProxy::testPassageRate()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, ME, false, Point(1000, 2000), 20000, TURN_NR);
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    // Initial request
    PassageInfoReceiver_t piReceiver;
    t.sig_passageChange.add(&piReceiver, &PassageInfoReceiver_t::onUpdate);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(piReceiver.get().distance, 141);

    // Update
    t.setPassageDistance(10);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(piReceiver.get().distance, 10);
    TS_ASSERT_DELTA(piReceiver.get().normalPassageRate, 0.90, 0.01);
    TS_ASSERT_DELTA(piReceiver.get().cloakedPassageRate, 0.95, 0.01);
}

/** Test getSweepInfo().
    A: create session with minefield. Create MinefieldProxy. Call getSweepInfo.
    E: correct value reported */
void
TestGameProxyMinefieldProxy::testSweepInfo()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, ME, false, Point(1000, 2000), 20000, TURN_NR);
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    // Request data
    MinefieldProxy::SweepInfo info;
    t.getSweepInfo(ind, info);

    // Verify
    TS_ASSERT_EQUALS(info.units, 19000);
    TS_ASSERT_EQUALS(info.isWeb, false);
    TS_ASSERT_EQUALS(info.weapons.size(), 11U);
    TS_ASSERT_EQUALS(info.weapons[0].needed, 4750);
    TS_ASSERT_EQUALS(info.weapons[0].name, "Laser");
    TS_ASSERT_EQUALS(info.weapons[9].needed, 48);
    TS_ASSERT_EQUALS(info.weapons[9].name, "Heavy Phaser");
    TS_ASSERT_EQUALS(info.weapons[10].needed, 950);
    TS_ASSERT_EQUALS(info.weapons[10].name, "Player 11 fighter");
}

/** Test addNewListener().
    A: create session with minefield. Create MinefieldProxy. Call addNewListener.
    E: listener is called */
void
TestGameProxyMinefieldProxy::testObjectListener()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, ME, false, Point(1000, 2000), 20000, TURN_NR);
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    // Let initial communication settle
    thread.sync();
    ind.processQueue();

    // Add listener
    int result = 0;
    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(int& result)
            : m_result(result)
            { }
        virtual void handle(game::Session&, game::map::Object* obj)
            {
                if (obj != 0) {
                    m_result = obj->getId();
                }
            }
     private:
        int& m_result;
    };
    t.addNewListener(new Listener(result));
    thread.sync();
    ind.processQueue();

    // Verify that listener has been called
    TS_ASSERT_EQUALS(result, 200);
}

/** Test browsing.
    A: create session with multiple minefields. Create MinefieldProxy. Call browse() functions.
    E: correct updates delivered */
void
TestGameProxyMinefieldProxy::testBrowse()
{
    using game::map::ObjectCursor;
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, ME, false, Point(1000, 2000), 20000, TURN_NR);
    addMinefield(thread, 300, ME, false, Point(1000, 2000), 20000, TURN_NR);
    addMinefield(thread, 400, ME, false, Point(1000, 2000), 20000, TURN_NR);
    addMinefield(thread, 500, ME, false, Point(1000, 2000), 20000, TURN_NR);
    thread.session().getGame()->currentTurn().universe().minefields().get(300)->setIsMarked(true);
    thread.session().getGame()->currentTurn().universe().minefields().get(500)->setIsMarked(true);
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    // Connect listeners
    MinefieldInfoReceiver_t miReceiver;
    t.sig_minefieldChange.add(&miReceiver, &MinefieldInfoReceiver_t::onUpdate);

    // Verify initial position
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 200);

    // Browse next
    t.browse(ObjectCursor::Next, false);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 300);

    // Browse previous
    t.browse(ObjectCursor::Previous, false);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 200);

    // Browse first marked
    t.browse(ObjectCursor::First, true);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 300);

    // Browse last
    t.browse(ObjectCursor::Last, false);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 500);

    // Browse previous
    t.browse(ObjectCursor::Previous, false);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 400);
}

/** Test browsing, special case: next marked with no marked units.
    A: create session with multiple minefields. Create MinefieldProxy. Call browse() functions.
    E: no change reported */
void
TestGameProxyMinefieldProxy::testBrowseUnmarked()
{
    using game::map::ObjectCursor;
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, ME, false, Point(1000, 2000), 20000, TURN_NR);
    addMinefield(thread, 300, ME, false, Point(1000, 2000), 20000, TURN_NR);
    addMinefield(thread, 400, ME, false, Point(1000, 2000), 20000, TURN_NR);
    addMinefield(thread, 500, ME, false, Point(1000, 2000), 20000, TURN_NR);
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    // Connect listeners
    MinefieldInfoReceiver_t miReceiver;
    t.sig_minefieldChange.add(&miReceiver, &MinefieldInfoReceiver_t::onUpdate);

    // Verify initial position
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 200);

    // Browse next marked -> still at 200
    t.browse(ObjectCursor::Next, true);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 200);
}

/** Test erase().
    A: create session with a minefield. Create MinefieldProxy. Call erase().
    E: must report the minefield first, then Id 0. */
void
TestGameProxyMinefieldProxy::testErase()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addMinefield(thread, 200, ME, false, Point(1000, 2000), 20000, TURN_NR);
    postprocessUniverse(thread);
    MinefieldProxy t(ind, thread.gameSender());

    // Connect listeners
    MinefieldInfoReceiver_t miReceiver;
    t.sig_minefieldChange.add(&miReceiver, &MinefieldInfoReceiver_t::onUpdate);

    // Receive initial request; verify initial position
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 200);

    // Erase
    t.erase(200);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(miReceiver.get().minefieldId, 0);
}

