/**
  *  \file u/t_game_map_messagelink.cpp
  *  \brief Test for game::map::MessageLink
  */

#include "game/map/messagelink.hpp"

#include "t_game_map.hpp"

/** Simple coverage test. */
void
TestGameMapMessageLink::testIt()
{
    game::map::MessageLink testee;
    TS_ASSERT(testee.empty());

    testee.add(2);
    testee.add(5);
    testee.add(5);
    testee.add(7);

    TS_ASSERT_EQUALS(testee.get().size(), 3U);
    TS_ASSERT_EQUALS(testee.get()[0], 2U);
    TS_ASSERT_EQUALS(testee.get()[1], 5U);
    TS_ASSERT_EQUALS(testee.get()[2], 7U);
    TS_ASSERT(!testee.empty());
}

