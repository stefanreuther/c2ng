/**
  *  \file u/t_game_proxy_referenceobserverproxy.cpp
  *  \brief Test for game::proxy::ReferenceObserverProxy
  */

#include "game/proxy/referenceobserverproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/test/sessionthread.hpp"
#include "game/turn.hpp"

namespace {
    using afl::base::Ptr;
    using game::Game;
    using game::map::IonStorm;

    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(afl::sys::Semaphore& sem, String_t& result)
            : m_sem(sem), m_result(result)
            { }

        virtual void handle(game::Session& s, game::map::Object* obj)
            {
                if (obj) {
                    m_result = obj->getName(game::PlainName, s.translator(), s.interface());
                    m_sem.post();
                }
            }

     private:
        afl::sys::Semaphore& m_sem;
        String_t& m_result;
    };
}

/** Simple test.
    A: create a universe with n objects in it, and a ReferenceObserverProxy eventually referring there. Add an observer.
    E: observer must see the provided object. Observer must be notified of changes. */
void
TestGameProxyReferenceObserverProxy::testIt()
{
    // Environment with two ion storms
    CxxTest::setAbortTestOnFail(true);
    game::test::SessionThread s;

    Ptr<Game> g = new Game();
    game::map::Universe& univ = g->currentTurn().universe();

    IonStorm& a = *univ.ionStorms().create(34);
    a.setName("Fred");
    a.setVoltage(100);
    a.setPosition(game::map::Point(1000, 2000));

    IonStorm& b = *univ.ionStorms().create(55);
    b.setName("Wilma");
    b.setVoltage(100);
    b.setPosition(game::map::Point(1000, 2000));

    s.session().setGame(g);

    // Tester
    afl::sys::Semaphore sem(0);
    String_t result;

    game::proxy::ReferenceObserverProxy testee(s.gameSender());
    testee.setReference(game::Reference(game::Reference::IonStorm, 34));

    // Add listener and wait for initial report
    testee.addNewListener(new Listener(sem, result));
    TS_ASSERT(sem.wait(1000));
    TS_ASSERT_EQUALS(result, "Fred");

    // Change to new object and wait for report
    testee.setReference(game::Reference(game::Reference::IonStorm, 55));
    TS_ASSERT(sem.wait(1000));
    TS_ASSERT_EQUALS(result, "Wilma");

    // Change object and wait for report
    b.setName("Betty");
    b.markDirty();         // FIXME: IonStorms currently do not mark themselves changed
    univ.notifyListeners();
    TS_ASSERT(sem.wait(1000));
    TS_ASSERT_EQUALS(result, "Betty");

    // Remove listeners because why not.
    // (This is just for coverage as removeAllListeners() is just a stop-gap function for now.)
    testee.removeAllListeners();
}

