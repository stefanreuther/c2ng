/**
  *  \file test/game/proxy/cursorobserverproxytest.cpp
  *  \brief Test for game::proxy::CursorObserverProxy
  */

#include "game/proxy/cursorobserverproxy.hpp"

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

    class CursorFactory : public game::map::ObjectCursorFactory {
     public:
        virtual game::map::ObjectCursor* getCursor(game::Session& session)
            {
                if (game::Game* g = session.getGame().get()) {
                    return &g->cursors().currentIonStorm();
                } else {
                    return 0;
                }
            }
    };

    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(afl::test::Assert a, afl::sys::Semaphore& sem, String_t& result)
            : m_assert(a), m_sem(sem), m_result(result)
            { }

        virtual void handle(game::Session& s, game::map::Object* obj)
            {
                m_assert.check("handle() obj", obj);

                m_result = obj->getName(game::PlainName, s.translator(), s.interface());
                m_sem.post();
            }

     private:
        afl::test::Assert m_assert;
        afl::sys::Semaphore& m_sem;
        String_t& m_result;
    };
}

/** Simple test.
    A: create a universe with an object in it, and a CursorObserverProxy eventually referring there. Add an observer.
    E: observer must see the provided object. */
AFL_TEST("game.proxy.CursorObserverProxy", a)
{
    // Environment
    game::test::SessionThread s;

    Ptr<Game> g = new Game();
    IonStorm& obj = *g->currentTurn().universe().ionStorms().create(34);
    obj.setName("Xaver");
    obj.setPosition(game::map::Point(1000, 2000));
    obj.setRadius(300);
    obj.setVoltage(50);
    g->cursors().currentIonStorm().setCurrentIndex(34);
    s.session().setGame(g);

    // Tester
    afl::sys::Semaphore sem(0);
    String_t result;

    game::proxy::CursorObserverProxy testee(s.gameSender(), std::auto_ptr<game::map::ObjectCursorFactory>(new CursorFactory()));
    testee.addNewListener(new Listener(a, sem, result));

    // Wait for response: must report correct value
    a.check("01. wait", sem.wait(1000));
    a.checkEqual("02. result", result, "Xaver");
}
