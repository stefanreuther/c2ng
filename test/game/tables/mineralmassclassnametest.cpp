/**
  *  \file test/game/tables/mineralmassclassnametest.cpp
  *  \brief Test for game::tables::MineralMassClassName
  */

#include "game/tables/mineralmassclassname.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.MineralMassClassName", a)
{
    afl::string::NullTranslator tx;
    game::tables::MineralMassClassName testee(tx);

    // Well-known values
    a.checkEqual("01", testee(10000), "abundant");
    a.checkEqual("02", testee(150), "rare");
    a.checkEqual("03", testee(0), "none");

    // Loop: 6 levels
    int n;
    int count = 0;
    String_t val;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        String_t val2 = testee(n);
        a.checkDifferent("11", val, val2);
        val = val2;
        ++count;
    }
    a.checkEqual("12", count, 6);
}
