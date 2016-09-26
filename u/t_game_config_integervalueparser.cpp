/**
  *  \file u/t_game_config_integervalueparser.cpp
  */

#include <stdexcept>
#include "game/config/integervalueparser.hpp"

#include "u/t_game_config.hpp"

void
TestGameConfigIntegerValueParser::testIt()
{
    // ex UtilConfTestSuite::testValueIntParser
    game::config::IntegerValueParser ivp;

    // Some random values
    TS_ASSERT_EQUALS(ivp.parse("0"), 0);
    TS_ASSERT_EQUALS(ivp.parse("1"), 1);
    TS_ASSERT_EQUALS(ivp.parse("65535"), 65535);
    TS_ASSERT_EQUALS(ivp.parse("65536"), 65536);
    TS_ASSERT_EQUALS(ivp.parse("2147483647"), 2147483647);
    TS_ASSERT_EQUALS(ivp.parse("-1"), -1);
    TS_ASSERT_EQUALS(ivp.parse("-2147483648"), int32_t(-2147483648U));

    // Spacing etc.
    TS_ASSERT_EQUALS(ivp.parse(" 42"), 42);
    TS_ASSERT_EQUALS(ivp.parse(" 42      "), 42);
    TS_ASSERT_EQUALS(ivp.parse("42        "), 42);

    // Wrong stuff
    TS_ASSERT_THROWS(ivp.parse("x"), std::range_error);
    TS_ASSERT_THROWS(ivp.parse("x42"), std::range_error);
    // no longer an exception, as we want to parse things like '100%':
    // TS_ASSERT_THROWS(ivp.parse("42x"), std::range_error);
    TS_ASSERT_EQUALS(ivp.parse("42x"), 42);
    TS_ASSERT_EQUALS(ivp.parse("100%"), 100);

    // Reverse conversion
    TS_ASSERT_EQUALS(ivp.toString(0), "0");
    TS_ASSERT_EQUALS(ivp.toString(1), "1");
    TS_ASSERT_EQUALS(ivp.toString(65535), "65535");
    TS_ASSERT_EQUALS(ivp.toString(65536), "65536");
    TS_ASSERT_EQUALS(ivp.toString(2147483647), "2147483647");
    TS_ASSERT_EQUALS(ivp.toString(-1), "-1");
    TS_ASSERT_EQUALS(ivp.toString(-2147483648U), "-2147483648");
}
