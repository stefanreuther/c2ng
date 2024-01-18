/**
  *  \file test/game/map/messagelinktest.cpp
  *  \brief Test for game::map::MessageLink
  */

#include "game/map/messagelink.hpp"
#include "afl/test/testrunner.hpp"

/** Simple coverage test. */
AFL_TEST("game.map.MessageLink", a)
{
    game::map::MessageLink testee;
    a.check("01. empty", testee.empty());

    testee.add(2);
    testee.add(5);
    testee.add(5);
    testee.add(7);

    a.checkEqual("11. size", testee.get().size(), 3U);
    a.checkEqual("12. index", testee.get()[0], 2U);
    a.checkEqual("13. index", testee.get()[1], 5U);
    a.checkEqual("14. index", testee.get()[2], 7U);
    a.check("15. empty", !testee.empty());
}
