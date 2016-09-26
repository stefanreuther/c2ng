/**
  *  \file u/t_game_playerarray.cpp
  *  \brief Test for game::PlayerArray
  */

#include "game/playerarray.hpp"

#include "t_game.hpp"

void
TestGamePlayerArray::testArray()
{
    // ex GamePlayerSetTestSuite::testArray()
    game::PlayerArray<int> n;

    // check indexing
    TS_ASSERT(!n.at(-2));
    TS_ASSERT(!n.at(-1));
    TS_ASSERT(n.at(0));
    TS_ASSERT(n.at(1));
    TS_ASSERT(n.at(10));
    TS_ASSERT(n.at(11));
    TS_ASSERT(n.at(12));
    TS_ASSERT(!n.at(-13));
    TS_ASSERT(!n.at(-14));
    TS_ASSERT(!n.at(1000));  // reconsider when we go MMORPG

    // check initialisation
    n.setAll(0);
    TS_ASSERT_EQUALS(*n.at(0), 0);
    TS_ASSERT_EQUALS(*n.at(1), 0);
    TS_ASSERT_EQUALS(*n.at(2), 0);
    TS_ASSERT_EQUALS(*n.at(10), 0);
    TS_ASSERT_EQUALS(*n.at(11), 0);
    TS_ASSERT_EQUALS(*n.at(12), 0);

    // check initialisation
    n.setAll(42);
    TS_ASSERT_EQUALS(*n.at(0), 42);
    TS_ASSERT_EQUALS(*n.at(1), 42);
    TS_ASSERT_EQUALS(*n.at(2), 42);
    TS_ASSERT_EQUALS(*n.at(10), 42);
    TS_ASSERT_EQUALS(*n.at(11), 42);
    TS_ASSERT_EQUALS(*n.at(12), 42);

    // check assignment
    n.set(2, 8);
    TS_ASSERT_EQUALS(*n.at(0), 42);
    TS_ASSERT_EQUALS(*n.at(1), 42);
    TS_ASSERT_EQUALS(*n.at(2), 8);
    TS_ASSERT_EQUALS(*n.at(3), 42);
    TS_ASSERT_EQUALS(*n.at(4), 42);

    // check modify-assignment [not relevant in c2ng]
    *n.at(2) += 7;
    TS_ASSERT_EQUALS(*n.at(0), 42);
    TS_ASSERT_EQUALS(*n.at(1), 42);
    TS_ASSERT_EQUALS(*n.at(2), 15);
    TS_ASSERT_EQUALS(*n.at(3), 42);
    TS_ASSERT_EQUALS(*n.at(4), 42);

    // check regular read
    TS_ASSERT_EQUALS(n.get(0), 42);
    TS_ASSERT_EQUALS(n.get(1), 42);
    TS_ASSERT_EQUALS(n.get(2), 15);
    TS_ASSERT_EQUALS(n.get(3), 42);
    TS_ASSERT_EQUALS(n.get(4), 42);

    // check out-of-bounds read
    TS_ASSERT_EQUALS(n.get(-1), 0);
    TS_ASSERT_EQUALS(n.get(999), 0);

    // check out-of-bounds write
    n.set(999999999, 9);
    n.set(-999999999, 9);
}
