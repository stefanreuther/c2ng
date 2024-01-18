/**
  *  \file test/game/config/integeroptiontest.cpp
  *  \brief Test for game::config::IntegerOption
  */

#include "game/config/integeroption.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/integervalueparser.hpp"

AFL_TEST("game.config.IntegerOption", a)
{
    game::config::IntegerValueParser vp;
    game::config::IntegerOption testee(vp, 9);

    // Verify initial state
    a.checkEqual("01. toString", testee.toString(), "9");
    a.checkEqual("02. value", testee(), 9);
    a.checkEqual("03. parser", &testee.parser(), &vp);

    // Modify
    testee.set(42);
    a.checkEqual("11. value", testee(), 42);

    testee.set("77");
    a.checkEqual("21. value", testee(), 77);

    testee.set("1 # with comment");
    a.checkEqual("31. value", testee(), 1);

    // Copying
    testee.copyFrom(game::config::IntegerOption(vp, 32));
    a.checkEqual("41. value", testee(), 32);
}
