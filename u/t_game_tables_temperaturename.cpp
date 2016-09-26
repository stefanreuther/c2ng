/**
  *  \file u/t_game_tables_temperaturename.cpp
  *  \brief Test for game::tables::TemperatureName
  */

#include "game/tables/temperaturename.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesTemperatureName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::TemperatureName testee(tx);

    // Well-known temperatures
    TS_ASSERT_EQUALS(testee(0), "arctic");
    TS_ASSERT_EQUALS(testee(14), "arctic");
    TS_ASSERT_EQUALS(testee(15), "cool");
    TS_ASSERT_EQUALS(testee(39), "cool");
    TS_ASSERT_EQUALS(testee(40), "warm");
    TS_ASSERT_EQUALS(testee(64), "warm");
    TS_ASSERT_EQUALS(testee(65), "tropical");
    TS_ASSERT_EQUALS(testee(84), "tropical");
    TS_ASSERT_EQUALS(testee(85), "desert");
    TS_ASSERT_EQUALS(testee(100), "desert");

    // Out of range
    TS_ASSERT_EQUALS(testee(-1), "arctic");
    TS_ASSERT_EQUALS(testee(-100), "arctic");
    TS_ASSERT_EQUALS(testee(101), "desert");
    TS_ASSERT_EQUALS(testee(1000), "desert");

    // Loop: 5 levels
    int n;
    String_t all;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        all += testee(n);
        all += " ";
    }
    TS_ASSERT_EQUALS(all, "arctic cool warm tropical desert ");
}

