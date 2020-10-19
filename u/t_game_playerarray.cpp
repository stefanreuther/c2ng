/**
  *  \file u/t_game_playerarray.cpp
  *  \brief Test for game::PlayerArray
  */

#include "game/playerarray.hpp"

#include "t_game.hpp"
#include "afl/string/string.hpp"

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

/** Test initialisation. */
void
TestGamePlayerArray::testInit()
{
    using game::PlayerArray;

    TS_ASSERT_EQUALS(PlayerArray<int>().get(1), 0);
    TS_ASSERT_EQUALS(PlayerArray<int>(42).get(1), 42);

    TS_ASSERT_EQUALS(PlayerArray<String_t>().get(1), "");
    TS_ASSERT_EQUALS(PlayerArray<String_t>("x").get(1), "x");
}

/** Test pointer handling.
    We want to safely receive null pointers when out of range. */
void
TestGamePlayerArray::testPointer()
{
    int a = 10, b = 20;
    game::PlayerArray<int*> n;
    n.set(3, &a);
    n.set(4, &b);

    TS_ASSERT(n.get(-1) == 0);
    TS_ASSERT(n.get(0) == 0);
    TS_ASSERT(n.get(3) == &a);
    TS_ASSERT(n.get(4) == &b);
    TS_ASSERT(n.get(1000) == 0);
}
