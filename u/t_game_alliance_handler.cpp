/**
  *  \file u/t_game_alliance_handler.cpp
  *  \brief Test for game::alliance::Handler
  */

#include "game/alliance/handler.hpp"

#include "t_game_alliance.hpp"

/** Interface test. */
void
TestGameAllianceHandler::testInterface()
{
    class Tester : public game::alliance::Handler {
     public:
        virtual void init(game::alliance::Container& /*allies*/, afl::string::Translator& /*tx*/)
            { }
        virtual void postprocess(game::alliance::Container& /*allies*/)
            { }
        virtual void handleChanges(const game::alliance::Container& /*allies*/)
            { }
    };
    Tester t;
}

