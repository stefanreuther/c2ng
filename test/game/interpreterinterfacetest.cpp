/**
  *  \file test/game/interpreterinterfacetest.cpp
  *  \brief Test for game::InterpreterInterface
  */

#include "game/interpreterinterface.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.InterpreterInterface")
{
    class Tester : public game::InterpreterInterface {
     public:
        virtual String_t getComment(Scope /*scope*/, int /*id*/) const
            { return String_t(); }
        virtual bool hasTask(Scope /*scope*/, int /*id*/) const
            { return false; }
        virtual afl::base::Optional<String_t> getHullShortName(int /*nr*/) const
            { return afl::base::Nothing; }
        virtual afl::base::Optional<String_t> getPlayerAdjective(int /*nr*/) const
            { return afl::base::Nothing; }
    };
    Tester t;
}
