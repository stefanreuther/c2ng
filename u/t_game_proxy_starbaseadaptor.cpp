/**
  *  \file u/t_game_proxy_starbaseadaptor.cpp
  *  \brief Test for game::proxy::StarbaseAdaptor
  */

#include "game/proxy/starbaseadaptor.hpp"

#include "t_game_proxy.hpp"

/** Interface test. */
void
TestGameProxyStarbaseAdaptor::testInterface()
{
    class Tester : public game::proxy::StarbaseAdaptor {
     public:
        virtual game::map::Planet& planet()
            { throw "no ref"; }
        virtual game::Session& session()
            { throw "no ref"; }
        virtual bool findShipCloningHere(game::Id_t& /*id*/, String_t& /*name*/)
            { return false; }
        virtual void cancelAllCloneOrders()
            { }
        virtual void notifyListeners()
            { }
    };
    Tester t;
}

