/**
  *  \file test/game/tables/wormholestabilitynametest.cpp
  *  \brief Test for game::tables::WormholeStabilityName
  */

#include "game/tables/wormholestabilityname.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.tables.WormholeStabilityName", a)
{
    afl::string::NullTranslator tx;
    game::tables::WormholeStabilityName testee(tx);

    // Well-known values
    a.checkEqual("01", testee(0), "very stable (<5%)");
    a.checkEqual("02", testee(5), "completely unstable");
    a.checkEqual("03", testee(5000), "completely unstable");

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
