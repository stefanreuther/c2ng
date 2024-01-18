/**
  *  \file test/game/interface/contextprovidertest.cpp
  *  \brief Test for game::interface::ContextProvider
  */

#include "game/interface/contextprovider.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.interface.ContextProvider")
{
    class Tester : public game::interface::ContextProvider {
     public:
        virtual void createContext(game::Session& /*session*/, interpreter::ContextReceiver& /*recv*/)
            { }
    };
    Tester t;
}
