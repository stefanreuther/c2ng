/**
  *  \file u/t_game_config_bitsetvalueparser.cpp
  */

#include "game/config/bitsetvalueparser.hpp"

#include "u/t_game_config.hpp"

void
TestGameConfigBitsetValueParser::testIt()
{
    // ex UtilConfTestSuite::testValueBitsetParser
    game::config::BitsetValueParser bvp("one,two,three,four,five");

    // Single values
    TS_ASSERT_EQUALS(bvp.parse(""), 0);
    TS_ASSERT_EQUALS(bvp.parse("one"), 1);
    TS_ASSERT_EQUALS(bvp.parse("two"), 2);
    TS_ASSERT_EQUALS(bvp.parse("three"), 4);
    TS_ASSERT_EQUALS(bvp.parse("four"), 8);
    TS_ASSERT_EQUALS(bvp.parse("five"), 16);

    // Multiple values
    TS_ASSERT_EQUALS(bvp.parse("one,two"), 3);
    TS_ASSERT_EQUALS(bvp.parse("two,three,four"), 14);
    TS_ASSERT_EQUALS(bvp.parse("five,three"), 20);
    TS_ASSERT_EQUALS(bvp.parse("one,one,one,one"), 1);
    TS_ASSERT_EQUALS(bvp.parse("five,,,,,,,,"), 16);

    // Numerical values
    TS_ASSERT_EQUALS(bvp.parse("one,120"), 121);
    TS_ASSERT_EQUALS(bvp.parse("one,121"), 121);
    TS_ASSERT_EQUALS(bvp.parse("121,one"), 121);

    // Reverse conversion
    TS_ASSERT_EQUALS(bvp.toString(0), "");
    TS_ASSERT_EQUALS(bvp.toString(1), "one");
    TS_ASSERT_EQUALS(bvp.toString(2), "two");
    TS_ASSERT_EQUALS(bvp.toString(3), "one,two");
    TS_ASSERT_EQUALS(bvp.toString(4), "three");
    TS_ASSERT_EQUALS(bvp.toString(32), "");
}
