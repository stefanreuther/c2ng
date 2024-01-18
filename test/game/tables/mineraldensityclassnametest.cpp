/**
  *  \file test/game/tables/mineraldensityclassnametest.cpp
  *  \brief Test for game::tables::MineralDensityClassName
  */

#include "game/tables/mineraldensityclassname.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.MineralDensityClassName", a)
{
    afl::string::NullTranslator tx;
    game::tables::MineralDensityClassName testee(tx);

    // Well-known values
    a.checkEqual("01", testee(25), "scattered");
    a.checkEqual("02", testee(33), "dispersed");
    a.checkEqual("03", testee(66), "concentrated");

    // Loop: 5 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    a.checkEqual("11", count, 5);
}
