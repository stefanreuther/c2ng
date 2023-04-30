/**
  *  \file u/t_game_config_stringarrayoption.cpp
  *  \brief Test for game::config::StringArrayOption
  */

#include "game/config/stringarrayoption.hpp"

#include "t_game_config.hpp"

/** Simple test. */
void
TestGameConfigStringArrayOption::testIt()
{
    game::config::StringArrayOption a(1, 10);
    TS_ASSERT_EQUALS(a.toString(), "");
    TS_ASSERT_EQUALS(a.getFirstIndex(), 1);
    TS_ASSERT_EQUALS(a.getNumSlots(), 10);
    TS_ASSERT_EQUALS(a(1), "");

    a.set("a, b, c");
    TS_ASSERT_EQUALS(a.toString(), "a,b,c");
    TS_ASSERT_EQUALS(a(0), "");
    TS_ASSERT_EQUALS(a(1), "a");
    TS_ASSERT_EQUALS(a(2), "b");
    TS_ASSERT_EQUALS(a(3), "c");
    TS_ASSERT_EQUALS(a(4), "");

    a.set(5, "x");
    TS_ASSERT_EQUALS(a.toString(), "a,b,c,,x");
    TS_ASSERT_EQUALS(a(3), "c");
    TS_ASSERT_EQUALS(a(4), "");
    TS_ASSERT_EQUALS(a(5), "x");
    TS_ASSERT_EQUALS(a(6), "");
}

