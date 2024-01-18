/**
  *  \file test/game/tables/basemissionnametest.cpp
  *  \brief Test for game::tables::BaseMissionName
  */

#include "game/tables/basemissionname.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.BaseMissionName", a)
{
    afl::string::NullTranslator tx;
    game::tables::BaseMissionName testee(tx);

    // Well-known values
    a.checkEqual("01", testee(0), "none");
    a.checkEqual("02", testee(5), "Repair base");

    // Does not crash
    testee(1000);

    // Loop: 7 values
    int n;
    int count = 0;
    String_t val;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        String_t val2 = testee(n);
        a.checkDifferent("11", val, val2);
        val = val2;
        ++count;
    }
    a.checkEqual("12", count, 7);
}
