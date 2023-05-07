/**
  *  \file game/test/interpreterinterface.cpp
  *  \brief Class game::test::InterpreterInterface
  */

#include "game/test/interpreterinterface.hpp"

String_t
game::test::InterpreterInterface::getComment(Scope /*scope*/, int /*id*/) const
{
    return String_t();
}

bool
game::test::InterpreterInterface::hasTask(Scope /*scope*/, int /*id*/) const
{
    return false;
}

afl::base::Optional<String_t>
game::test::InterpreterInterface::getHullShortName(int /*nr*/) const
{
    return afl::base::Nothing;
}

afl::base::Optional<String_t>
game::test::InterpreterInterface::getPlayerAdjective(int /*nr*/) const
{
    return afl::base::Nothing;
}
