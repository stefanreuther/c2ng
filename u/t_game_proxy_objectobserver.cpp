/**
  *  \file u/t_game_proxy_objectobserver.cpp
  *  \brief Test for game::proxy::ObjectObserver
  */

#include "game/proxy/objectobserver.hpp"

#include "t_game_proxy.hpp"

/** Interface test. */
void
TestGameProxyObjectObserver::testInterface()
{
    class Tester : public game::proxy::ObjectObserver {
     public:
        virtual void addNewListener(game::proxy::ObjectListener* /*pListener*/)
            { }
    };
    Tester t;
}

