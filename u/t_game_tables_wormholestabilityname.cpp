/**
  *  \file u/t_game_tables_wormholestabilityname.cpp
  *  \brief Test for game::tables::WormholeStabilityName
  */

#include "game/tables/wormholestabilityname.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesWormholeStabilityName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::WormholeStabilityName testee(tx);

    // Well-known values
    TS_ASSERT_EQUALS(testee(0), "very stable (<5%)");
    TS_ASSERT_EQUALS(testee(5), "completely unstable");
    TS_ASSERT_EQUALS(testee(5000), "completely unstable");

    // Loop: 6 levels
    int n;
    int count = 0;
    String_t val;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        String_t val2 = testee(n);
        TS_ASSERT_DIFFERS(val, val2);
        val = val2;
        ++count;
    }
    TS_ASSERT_EQUALS(count, 6);
}

