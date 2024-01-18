/**
  *  \file test/game/tables/nativegovernmentnametest.cpp
  *  \brief Test for game::tables::NativeGovernmentName
  */

#include "game/tables/nativegovernmentname.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.NativeGovernmentName", a)
{
    afl::string::NullTranslator tx;
    game::tables::NativeGovernmentName testee(tx);

    // In range
    a.checkEqual("01", testee(0), "none");
    a.checkEqual("02", testee(9), "Unity");

    // Out of range
    a.checkEqual("11", testee(-1), "?");
    a.checkEqual("12", testee(-100), "?");
    a.checkEqual("13", testee(10), "?");
    a.checkEqual("14", testee(1000), "?");

    // Loop: 9 levels + "none" = 10
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    a.checkEqual("21", count, 10);
}
