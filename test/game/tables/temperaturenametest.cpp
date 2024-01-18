/**
  *  \file test/game/tables/temperaturenametest.cpp
  *  \brief Test for game::tables::TemperatureName
  */

#include "game/tables/temperaturename.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.TemperatureName", a)
{
    afl::string::NullTranslator tx;
    game::tables::TemperatureName testee(tx);

    // Well-known temperatures
    a.checkEqual("01", testee(0), "arctic");
    a.checkEqual("02", testee(14), "arctic");
    a.checkEqual("03", testee(15), "cool");
    a.checkEqual("04", testee(39), "cool");
    a.checkEqual("05", testee(40), "warm");
    a.checkEqual("06", testee(64), "warm");
    a.checkEqual("07", testee(65), "tropical");
    a.checkEqual("08", testee(84), "tropical");
    a.checkEqual("09", testee(85), "desert");
    a.checkEqual("10", testee(100), "desert");

    // Out of range
    a.checkEqual("11", testee(-1), "arctic");
    a.checkEqual("12", testee(-100), "arctic");
    a.checkEqual("13", testee(101), "desert");
    a.checkEqual("14", testee(1000), "desert");

    // Loop: 5 levels
    int n;
    String_t all;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        all += testee(n);
        all += " ";
    }
    a.checkEqual("21", all, "arctic cool warm tropical desert ");
}
