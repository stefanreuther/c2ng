/**
  *  \file u/t_game_exception.cpp
  *  \brief Test for game::Exception
  */

#include <cstring>
#include "game/exception.hpp"

#include "t_game.hpp"

/** Simple test. */
void
TestGameException::testIt()
{
    {
        game::Exception testee("hi", "ho");
        TS_ASSERT_EQUALS(testee.getUserError(), "ho");
        TS_ASSERT_EQUALS(testee.getScriptError(), "hi");
        TS_ASSERT_EQUALS(std::strcmp(testee.what(), "ho"), 0);
    }
    {
        game::Exception testee("hurz");
        TS_ASSERT_EQUALS(testee.getUserError(), "hurz");
        TS_ASSERT_EQUALS(testee.getScriptError(), "hurz");
        TS_ASSERT_EQUALS(std::strcmp(testee.what(), "hurz"), 0);
    }
}

