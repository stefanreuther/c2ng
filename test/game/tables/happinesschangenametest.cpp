/**
  *  \file test/game/tables/happinesschangenametest.cpp
  *  \brief Test for game::tables::HappinessChangeName
  */

#include "game/tables/happinesschangename.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.HappinessChangeName", a)
{
    afl::string::NullTranslator tx;
    game::tables::HappinessChangeName testee(tx);

    // Well-known happinesses
    a.checkEqual("01", testee(0), "They are undecided about you.");
    a.checkEqual("02", testee(5), "They LOVE you.");
    a.checkEqual("03", testee(500), "They LOVE you.");
    a.checkEqual("04", testee(-6), "They HATE you!");
    a.checkEqual("05", testee(-500), "They HATE you!");

    // Loop: 5 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    a.checkEqual("11", count, 5);
}
