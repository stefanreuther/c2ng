/**
  *  \file test/game/proxy/objectlistenertest.cpp
  *  \brief Test for game::proxy::ObjectListener
  */

#include "game/proxy/objectlistener.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.proxy.ObjectListener")
{
    class Tester : public game::proxy::ObjectListener {
     public:
        virtual void handle(game::Session& /*s*/, game::map::Object* /*obj*/)
            { }
    };
    Tester t;
}
