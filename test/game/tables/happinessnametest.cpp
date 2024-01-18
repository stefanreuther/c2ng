/**
  *  \file test/game/tables/happinessnametest.cpp
  *  \brief Test for game::tables::HappinessName
  */

#include "game/tables/happinessname.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.HappinessName", a)
{
    afl::string::NullTranslator tx;
    game::tables::HappinessName testee(tx);

    // Well-known happinesses
    a.checkEqual("01", testee(100), "happy");
    a.checkEqual("02", testee(90), "happy");
    a.checkEqual("03", testee(70), "calm");
    a.checkEqual("04", testee(0), "fighting");
    a.checkEqual("05", testee(-300), "fighting");

    // Loop: 6 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    a.checkEqual("11", count, 6);
}
