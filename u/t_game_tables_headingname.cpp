/**
  *  \file u/t_game_tables_headingname.cpp
  *  \brief Test for game::tables::HeadingName
  */

#include "game/tables/headingname.hpp"

#include "t_game_tables.hpp"

void
TestGameTablesHeadingName::testIt()
{
    game::tables::HeadingName testee;

    // Well-known angles
    TS_ASSERT_EQUALS(testee(0), "N");
    TS_ASSERT_EQUALS(testee(90), "E");
    TS_ASSERT_EQUALS(testee(180), "S");
    TS_ASSERT_EQUALS(testee(270), "W");
    TS_ASSERT_EQUALS(testee(360), "N");

    // Loop: all angles
    int n;
    String_t all;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        all += testee(n);
        all += " ";
    }
    TS_ASSERT_EQUALS(all, "N NNE NE ENE E ESE SE SSE S SSW SW WSW W WNW NW NNW ");
}

