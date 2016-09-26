/**
  *  \file u/t_game_tables_basemissionname.cpp
  *  \brief Test for game::tables::BaseMissionName
  */

#include "game/tables/basemissionname.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesBaseMissionName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::BaseMissionName testee(tx);

    // Well-known values
    TS_ASSERT_EQUALS(testee(0), "none");
    TS_ASSERT_EQUALS(testee(5), "Repair base");

    // Does not crash
    testee(1000);

    // Loop: 7 values
    int n;
    int count = 0;
    String_t val;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        String_t val2 = testee(n);
        TS_ASSERT_DIFFERS(val, val2);
        val = val2;
        ++count;
    }
    TS_ASSERT_EQUALS(count, 7);
}

