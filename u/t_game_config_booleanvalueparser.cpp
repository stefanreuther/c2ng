/**
  *  \file u/t_game_config_booleanvalueparser.cpp
  */

#include "game/config/booleanvalueparser.hpp"

#include "u/t_game_config.hpp"

void
TestGameConfigBooleanValueParser::testIt()
{
    // ex UtilConfTestSuite::testValueBoolParser
    game::config::BooleanValueParser bvp;

    // Test some values
    TS_ASSERT_EQUALS(bvp.parse("no"), 0);
    TS_ASSERT_EQUALS(bvp.parse("yes"), 1);
    TS_ASSERT_EQUALS(bvp.parse("allies"), 2);
    TS_ASSERT_EQUALS(bvp.parse("external"), 3);
    TS_ASSERT_EQUALS(bvp.parse("true"), 1);
    TS_ASSERT_EQUALS(bvp.parse("false"), 0);

    // Short forms
    TS_ASSERT_EQUALS(bvp.parse("n"), 0);
    TS_ASSERT_EQUALS(bvp.parse("y"), 1);
    TS_ASSERT_EQUALS(bvp.parse("a"), 2);
    TS_ASSERT_EQUALS(bvp.parse("e"), 3);
    TS_ASSERT_EQUALS(bvp.parse("t"), 1);
    TS_ASSERT_EQUALS(bvp.parse("f"), 0);

    // Case
    TS_ASSERT_EQUALS(bvp.parse("NO"), 0);
    TS_ASSERT_EQUALS(bvp.parse("YES"), 1);
    TS_ASSERT_EQUALS(bvp.parse("ALL"), 2);
    TS_ASSERT_EQUALS(bvp.parse("EXT"), 3);
    TS_ASSERT_EQUALS(bvp.parse("TRU"), 1);
    TS_ASSERT_EQUALS(bvp.parse("FAL"), 0);

    // Numeric
    TS_ASSERT_EQUALS(bvp.parse("0"), 0);
    TS_ASSERT_EQUALS(bvp.parse("1"), 1);
    TS_ASSERT_EQUALS(bvp.parse("10"), 10);

    // Error, treated as 1
    TS_ASSERT_EQUALS(bvp.parse("Whateverest"), 1);

    // Reverse conversion
    TS_ASSERT_EQUALS(bvp.toString(0), "No");
    TS_ASSERT_EQUALS(bvp.toString(1), "Yes");
    TS_ASSERT_EQUALS(bvp.toString(2), "Allies");
    TS_ASSERT_EQUALS(bvp.toString(3), "External");
}
