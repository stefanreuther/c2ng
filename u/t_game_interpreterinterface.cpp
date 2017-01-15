/**
  *  \file u/t_game_interpreterinterface.cpp
  *  \brief Test for game::InterpreterInterface
  */

#include "game/interpreterinterface.hpp"

#include "t_game.hpp"

/** Interface test. */
void
TestGameInterpreterInterface::testIt()
{
    class Tester : public game::InterpreterInterface {
     public:
        virtual afl::data::Value* evaluate(Scope /*scope*/, int /*id*/, String_t /*expr*/)
            { return 0; }
        virtual String_t getComment(Scope /*scope*/, int /*id*/)
            { return String_t(); }
        virtual bool hasTask(Scope /*scope*/, int /*id*/)
            { return false; }
        virtual bool getHullShortName(int /*nr*/, String_t& /*out*/)
            { return false; }
        virtual bool getPlayerAdjective(int /*nr*/, String_t& /*out*/)
            { return false; }
    };
    Tester t;
}

