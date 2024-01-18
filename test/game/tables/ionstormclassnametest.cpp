/**
  *  \file test/game/tables/ionstormclassnametest.cpp
  *  \brief Test for game::tables::IonStormClassName
  */

#include "game/tables/ionstormclassname.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.IonStormClassName", a)
{
    afl::string::NullTranslator tx;
    game::tables::IonStormClassName testee(tx);

    // Well-known values
    a.checkEqual("01", testee(0), "harmless");
    a.checkEqual("02", testee(50), "moderate");
    a.checkEqual("03", testee(100), "strong");
    a.checkEqual("04", testee(150), "dangerous");
    a.checkEqual("05", testee(200), "VERY dangerous");
    a.checkEqual("06", testee(20000), "VERY dangerous");

    // Loop: 5 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    a.checkEqual("11", count, 5);
}
