/**
  *  \file u/t_game_tables_mineralmassclassname.cpp
  *  \brief Test for game::tables::MineralMassClassName
  */

#include "game/tables/mineralmassclassname.hpp"

#include "t_game_tables.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameTablesMineralMassClassName::testIt()
{
    afl::string::NullTranslator tx;
    game::tables::MineralMassClassName testee(tx);

    // Well-known values
    TS_ASSERT_EQUALS(testee(10000), "abundant");
    TS_ASSERT_EQUALS(testee(150), "rare");
    TS_ASSERT_EQUALS(testee(0), "none");

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
