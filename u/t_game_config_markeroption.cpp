/**
  *  \file u/t_game_config_markeroption.cpp
  *  \brief Test for game::config::MarkerOption
  */

#include "game/config/markeroption.hpp"

#include "t_game_config.hpp"

void
TestGameConfigMarkerOption::testIt()
{
    // Check initial values
    game::config::MarkerOption testee(7, 3);
    TS_ASSERT_EQUALS(testee().markerKind, 7);
    TS_ASSERT_EQUALS(testee().color, 3);
    TS_ASSERT_EQUALS(testee().note, "");
    TS_ASSERT_EQUALS(testee.toString(), "7,3,");

    // Check typed setters
    testee.set(game::config::MarkerOption::Data(1, 2, "x"));
    TS_ASSERT_EQUALS(testee().markerKind, 1);
    TS_ASSERT_EQUALS(testee().color, 2);
    TS_ASSERT_EQUALS(testee().note, "x");
    TS_ASSERT_EQUALS(testee.toString(), "1,2,x");

    // Check public setter
    testee.set("4, 5, hi");
    TS_ASSERT_EQUALS(testee().markerKind, 4);
    TS_ASSERT_EQUALS(testee().color, 5);
    TS_ASSERT_EQUALS(testee().note, "hi");
    TS_ASSERT_EQUALS(testee.toString(), "4,5,hi");

    // Check invalid setter -> does not change anything
    testee.set("10000,4,x");
    TS_ASSERT_EQUALS(testee().markerKind, 4);
    testee.set("1,10000,x");
    TS_ASSERT_EQUALS(testee().markerKind, 4);
    testee.set("a,b,c");
    TS_ASSERT_EQUALS(testee().markerKind, 4);
    testee.set("1");
    TS_ASSERT_EQUALS(testee().markerKind, 4);
}

