/**
  *  \file test/game/proxy/objectobservertest.cpp
  *  \brief Test for game::proxy::ObjectObserver
  */

#include "game/proxy/objectobserver.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.proxy.ObjectObserver")
{
    class Tester : public game::proxy::ObjectObserver {
     public:
        virtual void addNewListener(game::proxy::ObjectListener* /*pListener*/)
            { }
    };
    Tester t;
}
