/**
  *  \file test/game/tables/nativeracenametest.cpp
  *  \brief Test for game::tables::NativeRaceName
  */

#include "game/tables/nativeracename.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.NativeRaceName", a)
{
    afl::string::NullTranslator tx;
    game::tables::NativeRaceName testee(tx);

    // In range
    a.checkEqual("01", testee(0), "none");
    a.checkEqual("02", testee(9), "Siliconoid");
    a.checkEqual("03", testee(15), "Gaseous");

    // Out of range
    a.checkEqual("11", testee(-1), "?");
    a.checkEqual("12", testee(-100), "?");
    a.checkEqual("13", testee(16), "?");
    a.checkEqual("14", testee(1000), "?");

    // Loop: 15 races + "none" = 16
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    a.checkEqual("21", count, 16);
}
