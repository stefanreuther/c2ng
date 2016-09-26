/**
  *  \file u/t_game_tables_nativeracename.cpp
  *  \brief Test for game::tables::NativeRaceName
  */

#include "game/tables/nativeracename.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesNativeRaceName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::NativeRaceName testee(tx);

    // In range
    TS_ASSERT_EQUALS(testee(0), "none");
    TS_ASSERT_EQUALS(testee(9), "Siliconoid");
    TS_ASSERT_EQUALS(testee(15), "Gaseous");

    // Out of range
    TS_ASSERT_EQUALS(testee(-1), "?");
    TS_ASSERT_EQUALS(testee(-100), "?");
    TS_ASSERT_EQUALS(testee(16), "?");
    TS_ASSERT_EQUALS(testee(1000), "?");

    // Loop: 15 races + "none" = 16
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    TS_ASSERT_EQUALS(count, 16);
}


