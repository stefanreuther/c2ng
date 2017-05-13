/**
  *  \file u/t_game_config_integerarrayoption.cpp
  *  \brief Test for game::config::IntegerArrayOption
  */

#include "game/config/integerarrayoption.hpp"

#include "t_game_config.hpp"
#include "game/config/integervalueparser.hpp"

void
TestGameConfigIntegerArrayOption::testIt()
{
    game::config::IntegerValueParser vp;
    game::config::IntegerArrayOption<5> one(vp);

    // Verify initial state
    TS_ASSERT_EQUALS(one.getArray().size(), 5U);
    TS_ASSERT_EQUALS(*one.getArray().at(0), 0);
    TS_ASSERT_EQUALS(*one.getArray().at(4), 0);
    TS_ASSERT_EQUALS(one.toString(), "0,0,0,0,0");
    TS_ASSERT_EQUALS(&one.parser(), &vp);
    TS_ASSERT_EQUALS(one(1), 0);
    TS_ASSERT(one.isAllTheSame());
    
    // Modify
    one.set("3,     1, 4, 1, 5");
    TS_ASSERT_EQUALS(one(1), 3);
    TS_ASSERT_EQUALS(one(2), 1);
    TS_ASSERT_EQUALS(one(3), 4);
    TS_ASSERT_EQUALS(one(4), 1);
    TS_ASSERT_EQUALS(one(5), 5);

    TS_ASSERT_EQUALS(one(0), 5);
    TS_ASSERT_EQUALS(one(6), 5);
    TS_ASSERT_EQUALS(one(1000), 5);
    TS_ASSERT_EQUALS(one(-1), 5);

    TS_ASSERT_EQUALS(one.toString(), "3,1,4,1,5");

    // Another one
    static const int32_t init[] = { 3, 2, 1, 6, 8 };
    game::config::IntegerArrayOption<5> two(vp, init);
    TS_ASSERT_EQUALS(two.toString(), "3,2,1,6,8");

    two.copyFrom(one);
    TS_ASSERT_EQUALS(one.toString(), "3,1,4,1,5");
}

