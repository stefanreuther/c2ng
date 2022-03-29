/**
  *  \file u/t_game_interface_contextprovider.cpp
  *  \brief Test for game::interface::ContextProvider
  */

#include "game/interface/contextprovider.hpp"

#include "t_game_interface.hpp"

/** Interface test. */
void
TestGameInterfaceContextProvider::testInterface()
{
    class Tester : public game::interface::ContextProvider {
     public:
        virtual void createContext(game::Session& /*session*/, interpreter::ContextReceiver& /*recv*/)
            { }
    };
    Tester t;
}

