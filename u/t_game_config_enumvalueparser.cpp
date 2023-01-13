/**
  *  \file u/t_game_config_enumvalueparser.cpp
  *  \brief Test for game::config::EnumValueParser
  */

#include <stdexcept>
#include "game/config/enumvalueparser.hpp"

#include "t_game_config.hpp"

/** Simple test. */
void
TestGameConfigEnumValueParser::testIt()
{
    game::config::EnumValueParser t("one,two,three");

    TS_ASSERT_EQUALS(t.parse("one"), 0);
    TS_ASSERT_EQUALS(t.parse("two"), 1);
    TS_ASSERT_EQUALS(t.parse("three"), 2);

    TS_ASSERT_EQUALS(t.parse("ONE"), 0);
    TS_ASSERT_EQUALS(t.parse("TWO"), 1);
    TS_ASSERT_EQUALS(t.parse("THREE"), 2);

    TS_ASSERT_THROWS(t.parse(""), std::exception);
    TS_ASSERT_THROWS(t.parse("on"), std::exception);
    TS_ASSERT_THROWS(t.parse("ones"), std::exception);
    TS_ASSERT_THROWS(t.parse("four"), std::exception);

    TS_ASSERT_EQUALS(t.toString(0), "one");
    TS_ASSERT_EQUALS(t.toString(1), "two");
    TS_ASSERT_EQUALS(t.toString(2), "three");
    TS_ASSERT_EQUALS(t.toString(3), "3");
    TS_ASSERT_EQUALS(t.toString(3000), "3000");
    TS_ASSERT_EQUALS(t.toString(2000000000), "2000000000");
    TS_ASSERT_EQUALS(t.toString(-1), "-1");

    TS_ASSERT_EQUALS(t.parse("3000"), 3000);
    TS_ASSERT_EQUALS(t.parse("3"), 3);
}

/** Another test. */
void
TestGameConfigEnumValueParser::testIt2()
{
    game::config::EnumValueParser t("One,Two,Three");

    TS_ASSERT_EQUALS(t.parse("one"), 0);
    TS_ASSERT_EQUALS(t.parse("two"), 1);
    TS_ASSERT_EQUALS(t.parse("three"), 2);

    TS_ASSERT_EQUALS(t.parse("ONE"), 0);
    TS_ASSERT_EQUALS(t.parse("TWO"), 1);
    TS_ASSERT_EQUALS(t.parse("THREE"), 2);

    TS_ASSERT_THROWS(t.parse(""), std::exception);

    TS_ASSERT_EQUALS(t.toString(0), "One");
    TS_ASSERT_EQUALS(t.toString(1), "Two");
    TS_ASSERT_EQUALS(t.toString(2), "Three");
    TS_ASSERT_EQUALS(t.toString(3), "3");
}
