/**
  *  \file u/t_game_config_collapsibleintegerarrayoption.cpp
  *  \brief Test for game::config::CollapsibleIntegerArrayOption
  */

#include "game/config/collapsibleintegerarrayoption.hpp"

#include "t_game_config.hpp"
#include "game/config/integervalueparser.hpp"

void
TestGameConfigCollapsibleIntegerArrayOption::testIt()
{
    game::config::IntegerValueParser p;
    game::config::CollapsibleIntegerArrayOption<4> testee(p);

    TS_ASSERT_EQUALS(testee(1), 0);
    TS_ASSERT(testee.isAllTheSame());
    TS_ASSERT_EQUALS(testee.toString(), "0");

    testee.set("1,2,3,4");
    TS_ASSERT_EQUALS(testee.toString(), "1,2,3,4");
    TS_ASSERT_EQUALS(testee(1), 1);
    TS_ASSERT_EQUALS(testee(2), 2);
    TS_ASSERT_EQUALS(testee(3), 3);
    TS_ASSERT_EQUALS(testee(4), 4);

    testee.set(2, 3);
    testee.set(4, 3);
    TS_ASSERT_EQUALS(testee.toString(), "1,3,3,3");
    TS_ASSERT_EQUALS(testee(1), 1);
    TS_ASSERT_EQUALS(testee(2), 3);
    TS_ASSERT_EQUALS(testee(3), 3);
    TS_ASSERT_EQUALS(testee(4), 3);

    testee.set(1, 3);
    TS_ASSERT_EQUALS(testee.toString(), "3");
    TS_ASSERT_EQUALS(testee(1), 3)
    TS_ASSERT_EQUALS(testee(2), 3);
    TS_ASSERT_EQUALS(testee(3), 3);
    TS_ASSERT_EQUALS(testee(4), 3);

    testee.set(9);
    TS_ASSERT_EQUALS(testee.toString(), "9");
    TS_ASSERT_EQUALS(testee(1), 9)
    TS_ASSERT_EQUALS(testee(2), 9);
    TS_ASSERT_EQUALS(testee(3), 9);
    TS_ASSERT_EQUALS(testee(4), 9);

    testee.set("4");
    TS_ASSERT_EQUALS(testee.toString(), "4");
    TS_ASSERT_EQUALS(testee(1), 4)
    TS_ASSERT_EQUALS(testee(2), 4);
    TS_ASSERT_EQUALS(testee(3), 4);
    TS_ASSERT_EQUALS(testee(4), 4);
}

