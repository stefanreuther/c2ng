/**
  *  \file u/t_game_proxy_ionstormproxy.cpp
  *  \brief Test for game::proxy::IonStormProxy
  */

#include "game/proxy/ionstormproxy.hpp"

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

using game::proxy::IonStormProxy;
using game::test::SessionThread;
using game::test::WaitIndicator;

namespace {
    void prepare(SessionThread& h)
    {
        // Game with TurnScoreList, TeamSettings
        afl::base::Ptr<game::Game> g = new game::Game();
        h.session().setGame(g);

        // Root with PlayerList, HostVersion, Configuration (required for postprocessUniverse)
        afl::base::Ptr<game::Root> r = new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
        h.session().setRoot(r);

        // Ship list (required for postprocessUniverse)
        afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
        h.session().setShipList(sl);
    }

    void addIonStorm(SessionThread& h, game::Id_t id, int x, int y)
    {
        game::map::IonStorm* st = h.session().getGame()->currentTurn().universe().ionStorms().create(id);
        st->setName("Daniel");
        st->setPosition(game::map::Point(x, y));
        st->setRadius(100);
        st->setVoltage(60);
        st->setSpeed(4);
        st->setHeading(135);
        st->setIsGrowing(true);
    }

    void addUninitializedIonStorm(SessionThread& h, game::Id_t id)
    {
        game::map::IonStorm* st = h.session().getGame()->currentTurn().universe().ionStorms().create(id);
        st->setVoltage(50);    // Voltage is required for the storm to be recognized
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
    typedef Receiver<IonStormProxy::IonStormInfo> IonStormInfoReceiver_t;
}

/** Test behaviour on empty session.
    A: create empty session. Create IonStormProxy.
    E: default data reported */
void
TestGameProxyIonStormProxy::testEmpty()
{
    WaitIndicator ind;
    SessionThread thread;
    IonStormProxy t(ind, thread.gameSender());

    IonStormInfoReceiver_t receiver;
    game::test::Counter counter;
    t.sig_stormChange.add(&receiver, &IonStormInfoReceiver_t::onUpdate);
    t.sig_stormChange.add(&counter, &game::test::Counter::increment);

    // Receive initial data
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT(counter.get() > 0);
    TS_ASSERT_EQUALS(receiver.get().stormId, 0);
}

/** Test behaviour on session with no ion storms.
    A: create empty session with game but no ion storms. Create IonStormProxy.
    E: default data reported */
void
TestGameProxyIonStormProxy::testNoStorms()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    postprocessUniverse(thread);
    IonStormProxy t(ind, thread.gameSender());

    IonStormInfoReceiver_t receiver;
    game::test::Counter counter;
    t.sig_stormChange.add(&receiver, &IonStormInfoReceiver_t::onUpdate);
    t.sig_stormChange.add(&counter, &game::test::Counter::increment);

    // Receive initial data
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT(counter.get() > 0);
    TS_ASSERT_EQUALS(receiver.get().stormId, 0);
}

/** Test behaviour on session with normal ion storms.
    A: create empty session with game and an ion storm. Create IonStormProxy.
    E: correct data reported */
void
TestGameProxyIonStormProxy::testNormal()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addIonStorm(thread, 35, 2500, 1400);
    postprocessUniverse(thread);
    IonStormProxy t(ind, thread.gameSender());

    IonStormInfoReceiver_t receiver;
    t.sig_stormChange.add(&receiver, &IonStormInfoReceiver_t::onUpdate);

    // Receive initial data
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(receiver.get().stormId, 35);
    TS_ASSERT_EQUALS(receiver.get().center.getX(), 2500);
    TS_ASSERT_EQUALS(receiver.get().center.getY(), 1400);
    TS_ASSERT_EQUALS(receiver.get().radius, 100);
    TS_ASSERT_EQUALS(receiver.get().voltage, 60);
    TS_ASSERT_EQUALS(receiver.get().speed, 4);
    TS_ASSERT_EQUALS(receiver.get().text[IonStormProxy::Radius], "100 ly");
    TS_ASSERT_EQUALS(receiver.get().text[IonStormProxy::Status], "growing");
    TS_ASSERT_EQUALS(receiver.get().text[IonStormProxy::ClassName], "Class 2 (moderate)");
    TS_ASSERT(!receiver.get().forecast.empty());
}

/** Test behaviour on session with uninitialized ion storms.
    A: create empty session with game and an uninitialized ion storm. Create IonStormProxy.
    E: textual data reported as "unknown" */
void
TestGameProxyIonStormProxy::testUninit()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addUninitializedIonStorm(thread, 35);
    postprocessUniverse(thread);
    IonStormProxy t(ind, thread.gameSender());

    IonStormInfoReceiver_t receiver;
    t.sig_stormChange.add(&receiver, &IonStormInfoReceiver_t::onUpdate);

    // Receive initial data
    thread.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(receiver.get().stormId, 35);
    TS_ASSERT_EQUALS(receiver.get().center.getX(), 0);
    TS_ASSERT_EQUALS(receiver.get().center.getY(), 0);
    TS_ASSERT_EQUALS(receiver.get().text[IonStormProxy::Radius], "unknown");
    TS_ASSERT(receiver.get().forecast.empty());
}

/** Test browse().
    A: create empty session with game and multiple storms. Call browse().
    E: correct data reported */
void
TestGameProxyIonStormProxy::testBrowse()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addIonStorm(thread, 1, 2500, 1400);
    addIonStorm(thread, 3, 2400, 1500);
    addIonStorm(thread, 5, 2300, 1600);
    addIonStorm(thread, 7, 2200, 1700);
    postprocessUniverse(thread);
    IonStormProxy t(ind, thread.gameSender());

    IonStormInfoReceiver_t receiver;
    t.sig_stormChange.add(&receiver, &IonStormInfoReceiver_t::onUpdate);

    // Receive initial position; must be #1
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().stormId, 1);

    // Browse to next
    t.browse(game::map::ObjectCursor::Next, false);
    thread.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(receiver.get().stormId, 3);
}

/** Test addNewListener().
    A: create session with game and ion storm. Create IonStormProxy. Call addNewListener.
    E: listener is called */
void
TestGameProxyIonStormProxy::testObjectListener()
{
    WaitIndicator ind;
    SessionThread thread;
    prepare(thread);
    addIonStorm(thread, 7, 2200, 1700);
    postprocessUniverse(thread);
    IonStormProxy t(ind, thread.gameSender());

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
