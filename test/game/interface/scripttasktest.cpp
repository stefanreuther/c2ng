/**
  *  \file test/game/interface/scripttasktest.cpp
  *  \brief Test for game::interface::ScriptTask
  */

#include "game/interface/scripttask.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.interface.ScriptTask")
{
    class Tester : public game::interface::ScriptTask {
     public:
        virtual void execute(uint32_t /*pgid*/, game::Session& /*session*/)
            { }
    };
    Tester t;
}
