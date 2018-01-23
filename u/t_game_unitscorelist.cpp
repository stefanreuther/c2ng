/**
  *  \file u/t_game_unitscorelist.cpp
  *  \brief Test for game::UnitScoreList
  */

#include "game/unitscorelist.hpp"

#include "t_game.hpp"

/** Simple tests. */
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

/** Test that a UnitScoreList is copyable. */
void
TestGameUnitScoreList::testCopy()
{
    int16_t v, t;

    // Make a list
    game::UnitScoreList testee;
    testee.set(1, 100, 9);

    // Copy it and verify that we can get the correct result
    game::UnitScoreList other(testee);
    TS_ASSERT(other.get(1, v, t));
    TS_ASSERT_EQUALS(v, 100);
    TS_ASSERT_EQUALS(t, 9);

    // Add a value
    other.set(4, 40, 4);
    TS_ASSERT(other.get(4, v, t));

    // Assigning the original cancels the new value
    other = testee;
    TS_ASSERT(!other.get(4, v, t));
}

/** Test merge(). */
void
TestGameUnitScoreList::testMerge()
{
    int16_t v, t;

    // Make a list
    game::UnitScoreList testee;
    testee.set(1, 100, 9);

    // Merge same turn
    testee.merge(1, 200, 9);
    TS_ASSERT(testee.get(1, v, t));
    TS_ASSERT_EQUALS(v, 200);
    TS_ASSERT_EQUALS(t, 9);

    // Merge older turn (ignored)
    testee.merge(1, 300, 4);
    TS_ASSERT(testee.get(1, v, t));
    TS_ASSERT_EQUALS(v, 200);
    TS_ASSERT_EQUALS(t, 9);

    // Merge newer turn
    testee.merge(1, 400, 11);
    TS_ASSERT(testee.get(1, v, t));
    TS_ASSERT_EQUALS(v, 400);
    TS_ASSERT_EQUALS(t, 11);
}

