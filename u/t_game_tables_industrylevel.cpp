/**
  *  \file u/t_game_tables_industrylevel.cpp
  *  \brief Test for game::tables::IndustryLevel
  */

#include "game/tables/industrylevel.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/types.hpp"

void
TestGameTablesIndustryLevel::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::IndustryLevel testee(tx);

    // Enum values
    TS_ASSERT_EQUALS(testee(game::MinimalIndustry), "minimal");
    TS_ASSERT_EQUALS(testee(game::LightIndustry), "light");
    TS_ASSERT_EQUALS(testee(game::ModerateIndustry), "moderate");
    TS_ASSERT_EQUALS(testee(game::SubstantialIndustry), "substantial");
    TS_ASSERT_EQUALS(testee(game::HeavyIndustry), "heavy");

    // Outside ranges
    TS_ASSERT_EQUALS(testee(game::MinimalIndustry-1), "minimal");
    TS_ASSERT_EQUALS(testee(game::MinimalIndustry-100), "minimal");
    TS_ASSERT_EQUALS(testee(game::HeavyIndustry+100), "heavy");
    TS_ASSERT_EQUALS(testee(game::HeavyIndustry+1), "heavy");

    // Loop: 5 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    TS_ASSERT_EQUALS(count, 5);
}
