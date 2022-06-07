/**
  *  \file u/t_game_proxy_ufoproxy.cpp
  *  \brief Test for game::proxy::UfoProxy
  */

#include "game/proxy/ufoproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/root.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using game::proxy::UfoProxy;
using game::test::SessionThread;
using game::test::WaitIndicator;

namespace {
    void prepare(SessionThread& h)
    {
        // Game with Universe
        afl::base::Ptr<game::Game> g = new game::Game();
        h.session().setGame(g);

        // Root with PlayerList, HostVersion, Configuration (required for postprocessUniverse)
        afl::base::Ptr<game::Root> r = new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
        h.session().setRoot(r);

        // Ship list (required for postprocessUniverse)
        afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
        h.session().setShipList(sl);
    }

    game::map::Ufo* addUfo(SessionThread& h, game::Id_t id, int x, int y)
    {
        game::map::Ufo* p = h.session().getGame()->currentTurn().universe().ufos().addUfo(id, 1, 3);
        p->setPosition(game::map::Point(x, y));
        p->setInfo1("One");
        p->setInfo2("Two");
        p->setRadius(20);
        p->setMovementVector(game::map::Point(1, 2));
        p->setPlanetRange(200);
        p->setShipRange(150);
        p->setColorCode(3);
        return p;
    }

    void addUninitializedUfo(SessionThread& h, game::Id_t id)
    {
        // game::map::Ufo* p =
            h.session().getGame()->currentTurn().universe().ufos().addUfo(id, 2,4);
        // p->setColorCode(4);    // Color is required for the Ufo to be recognized
    }

    void postprocessUniverse(SessionThread& h)
    {
        // postprocess() will set up the cursors, so that they sit on an object
        h.session().postprocessTurn(h.session().getGame()->currentTurn(),
                                    game::PlayerSet_t(),  // Playing
                                    game::PlayerSet_t(),  // Available
                                    game::map::Object::Playable);
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
    typedef Receiver<UfoProxy::UfoInfo> UfoInfoReceiver_t;
}

/** Test behaviour on empty session.
    A: create empty session. Create UfoProxy.
    E: default data reported */
void
TestGameProxyUfoProxy::testEmpty()
{
    WaitIndicator ind;
    SessionThread thread;
    UfoProxy t(ind, thread.gameSender());

    UfoInfoReceiver_t receiver;
    game::test::Counter counter;
    t.sig_ufoChange.add(&receiver, &UfoInfoReceiver_t::onUpdate);
    t.sig_ufoChange.add(&counter, &game::test::Counter::increment);

    // Receive initial data
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT(counter.get() > 0);
    TS_ASSERT_EQUALS(receiver.get().ufoId, 0);
}

/** Test behaviour on session with no ufos.
    A: create empty session with game but no ufos. Create UfoProxy.
    E: default data reported */
void
TestGameProxyUfoProxy::testNoUfos()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    postprocessUniverse(thread);
    UfoProxy t(ind, thread.gameSender());

    UfoInfoReceiver_t receiver;
    game::test::Counter counter;
    t.sig_ufoChange.add(&receiver, &UfoInfoReceiver_t::onUpdate);
    t.sig_ufoChange.add(&counter, &game::test::Counter::increment);

    // Receive initial data
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT(counter.get() > 0);
    TS_ASSERT_EQUALS(receiver.get().ufoId, 0);
}

/** Test behaviour on session with normal ufo.
    A: create empty session with game and an ufo. Create UfoProxy.
    E: correct data reported */
void
TestGameProxyUfoProxy::testNormal()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addUfo(thread, 35, 2500, 1400);
    postprocessUniverse(thread);
    UfoProxy t(ind, thread.gameSender());

    UfoInfoReceiver_t receiver;
    t.sig_ufoChange.add(&receiver, &UfoInfoReceiver_t::onUpdate);

    // Receive initial data
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(receiver.get().ufoId, 35);
    TS_ASSERT_EQUALS(receiver.get().center.getX(), 2500);
    TS_ASSERT_EQUALS(receiver.get().center.getY(), 1400);
    TS_ASSERT_EQUALS(receiver.get().radius, 20);
    TS_ASSERT_EQUALS(receiver.get().text[UfoProxy::Info1], "One");
    TS_ASSERT_EQUALS(receiver.get().text[UfoProxy::PlanetRange], "200 ly");
    TS_ASSERT_EQUALS(receiver.get().text[UfoProxy::Heading], "unknown (+1,+2)");
}

/** Test behaviour on session with uninitialized Ufos.
    A: create empty session with game and an uninitialized Ufo. Create UfoProxy.
    E: textual data reported as "unknown" */
void
TestGameProxyUfoProxy::testUnknown()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addUninitializedUfo(thread, 35);
    postprocessUniverse(thread);
    UfoProxy t(ind, thread.gameSender());

    UfoInfoReceiver_t receiver;
    t.sig_ufoChange.add(&receiver, &UfoInfoReceiver_t::onUpdate);

    // Receive initial data
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(receiver.get().ufoId, 35);
    TS_ASSERT_EQUALS(receiver.get().center.getX(), 0);
    TS_ASSERT_EQUALS(receiver.get().center.getY(), 0);
    TS_ASSERT_EQUALS(receiver.get().text[UfoProxy::Radius], "unknown");
}

/** Test browse().
    A: create empty session with game and multiple Ufos. Call browse().
    E: correct data reported */
void
TestGameProxyUfoProxy::testBrowse()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addUfo(thread, 1, 2500, 1400);
    addUfo(thread, 3, 2400, 1500);
    addUfo(thread, 5, 2300, 1600);
    postprocessUniverse(thread);
    UfoProxy t(ind, thread.gameSender());

    UfoInfoReceiver_t receiver;
    t.sig_ufoChange.add(&receiver, &UfoInfoReceiver_t::onUpdate);

    // Receive initial position; must be #1
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().ufoId, 1);

    // Browse to next
    t.browse(game::map::ObjectCursor::Next, false);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().ufoId, 3);

    // Browse to other end has no effect as Ufos are not connected
    t.browseToOtherEnd();
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().ufoId, 3);
}

/** Test addNewListener().
    A: create session with game and Ufo. Create UfoProxy. Call addNewListener.
    E: listener is called */
void
TestGameProxyUfoProxy::testObjectListener()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addUfo(thread, 7, 2200, 1700);
    postprocessUniverse(thread);
    UfoProxy t(ind, thread.gameSender());

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
    TS_ASSERT_EQUALS(result, 7);
}

/** Test toggleStoredInHistory().
    A: create session with game and Ufo. Create UfoProxy. Call toggleStoredInHistory.
    E: value updated and reported correctly */
void
TestGameProxyUfoProxy::testToggle()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    game::map::Ufo* p = addUfo(thread, 1, 2500, 1400);
    postprocessUniverse(thread);
    UfoProxy t(ind, thread.gameSender());

    UfoInfoReceiver_t receiver;
    t.sig_ufoChange.add(&receiver, &UfoInfoReceiver_t::onUpdate);

    // Receive initial update: not stored in history
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().isStoredInHistory, false);

    // Toggle on
    t.toggleStoredInHistory();
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().isStoredInHistory, true);
    TS_ASSERT_EQUALS(p->isStoredInHistory(), true);

    // Toggle off
    t.toggleStoredInHistory();
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().isStoredInHistory, false);
    TS_ASSERT_EQUALS(p->isStoredInHistory(), false);
}

/** Test browseToOtherEnd().
    A: create session with game and connected Ufos. Create UfoProxy. Call browseToOtherEnd.
    E: other Id reported correctly */
void
TestGameProxyUfoProxy::testBrowseOtherEnd()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    game::map::Ufo* p1 = addUfo(thread, 1, 2500, 1400);
    game::map::Ufo* p2 = addUfo(thread, 2, 2500, 1400);
    game::map::Ufo* p3 = addUfo(thread, 3, 2500, 1400);
    game::map::Ufo* p4 = addUfo(thread, 4, 2500, 1400);
    p1->setName("First");
    p2->setName("Second");
    p3->setName("Third");
    p4->setName("Fourth");
    p1->setInfo1("Info First");
    p2->setInfo1("Info Second");
    p3->setInfo1("Info Third");
    p4->setInfo1("Info Fourth");
    p1->connectWith(*p3);
    postprocessUniverse(thread);
    UfoProxy t(ind, thread.gameSender());

    UfoInfoReceiver_t receiver;
    t.sig_ufoChange.add(&receiver, &UfoInfoReceiver_t::onUpdate);

    // Verify initial state
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().ufoId, 1);
    TS_ASSERT_EQUALS(receiver.get().text[UfoProxy::Info1], "Info First");
    TS_ASSERT_EQUALS(receiver.get().text[UfoProxy::OtherEndName], "Ufo #3: Third");

    // Browse to other end
    t.browseToOtherEnd();
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().ufoId, 3);
    TS_ASSERT_EQUALS(receiver.get().text[UfoProxy::Info1], "Info Third");
    TS_ASSERT_EQUALS(receiver.get().text[UfoProxy::OtherEndName], "Ufo #1: First");

    // Browse back
    t.browseToOtherEnd();
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().ufoId, 1);
}

