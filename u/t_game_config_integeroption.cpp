/**
  *  \file u/t_game_config_integeroption.cpp
  *  \brief Test for game::config::IntegerOption
  */

#include "game/config/integeroption.hpp"

#include "t_game_config.hpp"
#include "game/config/integervalueparser.hpp"

/** Test it. */
void
TestGameConfigIntegerOption::testIt()
{
    game::config::IntegerValueParser vp;
    game::config::IntegerOption testee(vp, 9);

    // Verify initial state
    TS_ASSERT_EQUALS(testee.toString(), "9");
    TS_ASSERT_EQUALS(testee(), 9);
    TS_ASSERT_EQUALS(&testee.parser(), &vp);

    // Modify
    testee.set(42);
    TS_ASSERT_EQUALS(testee(), 42);

    testee.set("77");
    TS_ASSERT_EQUALS(testee(), 77);

    testee.set("1 # with comment");
    TS_ASSERT_EQUALS(testee(), 1);

    // Copying
    testee.copyFrom(game::config::IntegerOption(vp, 32));
    TS_ASSERT_EQUALS(testee(), 32);
}
