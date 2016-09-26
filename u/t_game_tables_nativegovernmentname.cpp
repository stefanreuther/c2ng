/**
  *  \file u/t_game_tables_nativegovernmentname.cpp
  *  \brief Test for game::tables::NativeGovernmentName
  */

#include "game/tables/nativegovernmentname.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesNativeGovernmentName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::NativeGovernmentName testee(tx);

    // In range
    TS_ASSERT_EQUALS(testee(0), "none");
    TS_ASSERT_EQUALS(testee(9), "Unity");

    // Out of range
    TS_ASSERT_EQUALS(testee(-1), "?");
    TS_ASSERT_EQUALS(testee(-100), "?");
    TS_ASSERT_EQUALS(testee(10), "?");
    TS_ASSERT_EQUALS(testee(1000), "?");

    // Loop: 9 levels + "none" = 10
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    TS_ASSERT_EQUALS(count, 10);
}
