/**
  *  \file test/game/proxy/referenceobserverproxytest.cpp
  *  \brief Test for game::proxy::ReferenceObserverProxy
  */

#include "game/proxy/referenceobserverproxy.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.proxy.ReferenceObserverProxy", a)
{
    // Environment with two ion storms
    game::test::SessionThread s;

    Ptr<Game> g = new Game();
    game::map::Universe& univ = g->currentTurn().universe();

    IonStorm& ia = *univ.ionStorms().create(34);
    ia.setName("Fred");
    ia.setVoltage(100);
    ia.setPosition(game::map::Point(1000, 2000));

    IonStorm& ib = *univ.ionStorms().create(55);
    ib.setName("Wilma");
    ib.setVoltage(100);
    ib.setPosition(game::map::Point(1000, 2000));

    s.session().setGame(g);

    // Tester
    afl::sys::Semaphore sem(0);
    String_t result;

    game::proxy::ReferenceObserverProxy testee(s.gameSender());
    testee.setReference(game::Reference(game::Reference::IonStorm, 34));

    // Add listener and wait for initial report
    testee.addNewListener(new Listener(sem, result));
    a.check("01. wait", sem.wait(1000));
    a.checkEqual("02. result", result, "Fred");

    // Change to new object and wait for report
    testee.setReference(game::Reference(game::Reference::IonStorm, 55));
    a.check("11. wait", sem.wait(1000));
    a.checkEqual("12. result", result, "Wilma");

    // Change object and wait for report
    ib.setName("Betty");
    ib.markDirty();         // FIXME: IonStorms currently do not mark themselves changed
    univ.notifyListeners();
    a.check("21. wait", sem.wait(1000));
    a.checkEqual("22. result", result, "Betty");

    // Remove listeners because why not.
    // (This is just for coverage as removeAllListeners() is just a stop-gap function for now.)
    testee.removeAllListeners();
}
