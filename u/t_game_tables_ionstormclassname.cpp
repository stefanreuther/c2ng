/**
  *  \file u/t_game_tables_ionstormclassname.cpp
  *  \brief Test for game::tables::IonStormClassName
  */

#include "game/tables/ionstormclassname.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesIonStormClassName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::IonStormClassName testee(tx);

    // Well-known values
    TS_ASSERT_EQUALS(testee(0), "harmless");
    TS_ASSERT_EQUALS(testee(50), "moderate");
    TS_ASSERT_EQUALS(testee(100), "strong");
    TS_ASSERT_EQUALS(testee(150), "dangerous");
    TS_ASSERT_EQUALS(testee(200), "VERY dangerous");
    TS_ASSERT_EQUALS(testee(20000), "VERY dangerous");

    // Loop: 5 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    TS_ASSERT_EQUALS(count, 5);
}
