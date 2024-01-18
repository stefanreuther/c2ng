/**
  *  \file test/game/tables/headingnametest.cpp
  *  \brief Test for game::tables::HeadingName
  */

#include "game/tables/headingname.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.HeadingName", a)
{
    game::tables::HeadingName testee;

    // Well-known angles
    a.checkEqual("01", testee(0), "N");
    a.checkEqual("02", testee(90), "E");
    a.checkEqual("03", testee(180), "S");
    a.checkEqual("04", testee(270), "W");
    a.checkEqual("05", testee(360), "N");

    // Loop: all angles
    int n;
    String_t all;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        all += testee(n);
        all += " ";
    }
    a.checkEqual("11", all, "N NNE NE ENE E ESE SE SSE S SSW SW WSW W WNW NW NNW ");
}
