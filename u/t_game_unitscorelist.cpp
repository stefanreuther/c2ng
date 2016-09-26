/**
  *  \file u/t_game_unitscorelist.cpp
  *  \brief Test for game::UnitScoreList
  */

#include "game/unitscorelist.hpp"

#include "t_game.hpp"

void
TestGameUnitScoreList::testIt()
{
    game::UnitScoreList testee;
    int16_t v, t;
    TS_ASSERT(!testee.get(1, v, t));

    testee.set(1, 20, 10);
    TS_ASSERT(testee.get(1, v, t));
    TS_ASSERT_EQUALS(v, 20);
    TS_ASSERT_EQUALS(t, 10);

    TS_ASSERT(!testee.get(0, v, t));
    TS_ASSERT(!testee.get(2, v, t));

    testee.merge(1, 20, 5);
    TS_ASSERT(testee.get(1, v, t));
    TS_ASSERT_EQUALS(v, 20);
    TS_ASSERT_EQUALS(t, 10);

    testee.merge(3, 33, 3);
    TS_ASSERT(testee.get(3, v, t));
    TS_ASSERT_EQUALS(v, 33);
    TS_ASSERT_EQUALS(t, 3);
}
