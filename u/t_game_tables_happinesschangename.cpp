/**
  *  \file u/t_game_tables_happinesschangename.cpp
  *  \brief Test for game::tables::HappinessChangeName
  */

#include "game/tables/happinesschangename.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesHappinessChangeName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::HappinessChangeName testee(tx);

    // Well-known happinesses
    TS_ASSERT_EQUALS(testee(0), "They are undecided about you.");
    TS_ASSERT_EQUALS(testee(5), "They LOVE you.");
    TS_ASSERT_EQUALS(testee(500), "They LOVE you.");
    TS_ASSERT_EQUALS(testee(-6), "They HATE you!");
    TS_ASSERT_EQUALS(testee(-500), "They HATE you!");

    // Loop: 5 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    TS_ASSERT_EQUALS(count, 5);
}
