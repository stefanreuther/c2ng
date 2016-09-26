/**
  *  \file u/t_game_tables_mineraldensityclassname.cpp
  *  \brief Test for game::tables::MineralDensityClassName
  */

#include "game/tables/mineraldensityclassname.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesMineralDensityClassName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::MineralDensityClassName testee(tx);

    // Well-known values
    TS_ASSERT_EQUALS(testee(25), "scattered");
    TS_ASSERT_EQUALS(testee(33), "dispersed");
    TS_ASSERT_EQUALS(testee(66), "concentrated");

    // Loop: 5 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    TS_ASSERT_EQUALS(count, 5);
}
