/**
  *  \file u/t_game_config_costarrayoption.cpp
  *  \brief Test for game::config::CostArrayOption
  */

#include "game/config/costarrayoption.hpp"

#include "t_game_config.hpp"

/** Test set(), case 1. */
void
TestGameConfigCostArrayOption::testSet1()
{
    game::config::CostArrayOption testee;
    TS_ASSERT(!testee.isChanged());

    testee.set("T10 D20 M30");
    TS_ASSERT(testee.isChanged());
    TS_ASSERT_EQUALS(testee(1).get(game::spec::Cost::Tritanium), 10);
    TS_ASSERT_EQUALS(testee(1).get(game::spec::Cost::Duranium), 20);
    TS_ASSERT_EQUALS(testee(1).get(game::spec::Cost::Molybdenum), 30);

    TS_ASSERT_EQUALS(testee(10).get(game::spec::Cost::Tritanium), 10);
    TS_ASSERT_EQUALS(testee(10).get(game::spec::Cost::Duranium), 20);
    TS_ASSERT_EQUALS(testee(10).get(game::spec::Cost::Molybdenum), 30);

    TS_ASSERT_EQUALS(testee.toString(), "T10 D20 M30");
}

/** Test set(), case 2. */
void
TestGameConfigCostArrayOption::testSet2()
{
    game::config::CostArrayOption testee;
    testee.set("T10,T20,T30,T40,T50");
    TS_ASSERT_EQUALS(testee(1).get(game::spec::Cost::Tritanium), 10);
    TS_ASSERT_EQUALS(testee(1).get(game::spec::Cost::Duranium), 0);
    TS_ASSERT_EQUALS(testee(1).get(game::spec::Cost::Molybdenum), 0);

    TS_ASSERT_EQUALS(testee(2).get(game::spec::Cost::Tritanium), 20);
    TS_ASSERT_EQUALS(testee(2).get(game::spec::Cost::Duranium), 0);
    TS_ASSERT_EQUALS(testee(2).get(game::spec::Cost::Molybdenum), 0);

    TS_ASSERT_EQUALS(testee(5).get(game::spec::Cost::Tritanium), 50);
    TS_ASSERT_EQUALS(testee(5).get(game::spec::Cost::Duranium), 0);
    TS_ASSERT_EQUALS(testee(5).get(game::spec::Cost::Molybdenum), 0);

    TS_ASSERT_EQUALS(testee(6).get(game::spec::Cost::Tritanium), 50);
    TS_ASSERT_EQUALS(testee(6).get(game::spec::Cost::Duranium), 0);
    TS_ASSERT_EQUALS(testee(6).get(game::spec::Cost::Molybdenum), 0);

    TS_ASSERT_EQUALS(testee(10).get(game::spec::Cost::Tritanium), 50);
    TS_ASSERT_EQUALS(testee(10).get(game::spec::Cost::Duranium), 0);
    TS_ASSERT_EQUALS(testee(10).get(game::spec::Cost::Molybdenum), 0);
}

/** Test set(), case 2. */
void
TestGameConfigCostArrayOption::testSet3()
{
    game::config::CostArrayOption testee;
    testee.set("T10");
    testee.set(2, game::spec::Cost::fromString("M5"));
    
    TS_ASSERT_EQUALS(testee(1).get(game::spec::Cost::Tritanium), 10);
    TS_ASSERT_EQUALS(testee(1).get(game::spec::Cost::Duranium), 0);
    TS_ASSERT_EQUALS(testee(1).get(game::spec::Cost::Molybdenum), 0);

    TS_ASSERT_EQUALS(testee(2).get(game::spec::Cost::Tritanium), 0);
    TS_ASSERT_EQUALS(testee(2).get(game::spec::Cost::Duranium), 0);
    TS_ASSERT_EQUALS(testee(2).get(game::spec::Cost::Molybdenum), 5);

    TS_ASSERT_EQUALS(testee(3).get(game::spec::Cost::Tritanium), 10);
    TS_ASSERT_EQUALS(testee(3).get(game::spec::Cost::Duranium), 0);
    TS_ASSERT_EQUALS(testee(3).get(game::spec::Cost::Molybdenum), 0);
}

