/**
  *  \file u/t_game_tables_happinessname.cpp
  *  \brief Test for game::tables::HappinessName
  */

#include "game/tables/happinessname.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesHappinessName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::HappinessName testee(tx);

    // Well-known happinesses
    TS_ASSERT_EQUALS(testee(100), "happy");
    TS_ASSERT_EQUALS(testee(90), "happy");
    TS_ASSERT_EQUALS(testee(70), "calm");
    TS_ASSERT_EQUALS(testee(0), "fighting");
    TS_ASSERT_EQUALS(testee(-300), "fighting");

    // Loop: 6 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    TS_ASSERT_EQUALS(count, 6);
}

