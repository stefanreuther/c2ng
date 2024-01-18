/**
  *  \file test/game/alliance/handlertest.cpp
  *  \brief Test for game::alliance::Handler
  */

#include "game/alliance/handler.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.alliance.Handler")
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
