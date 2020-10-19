/**
  *  \file u/t_game_proxy_objectlistener.cpp
  *  \brief Test for game::proxy::ObjectListener
  */

#include "game/proxy/objectlistener.hpp"

#include "t_game_proxy.hpp"

/** Interface test. */
void
TestGameProxyObjectListener::testInterface()
{
    class Tester : public game::proxy::ObjectListener {
     public:
        virtual void handle(game::Session& /*s*/, game::map::Object* /*obj*/)
            { }
    };
    Tester t;
}

