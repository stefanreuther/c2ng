/**
  *  \file u/t_game_map_basestorage.cpp
  *  \brief Test for game::map::BaseStorage
  */

#include "game/map/basestorage.hpp"

#include "t_game_map.hpp"

/** Test general element access. */
void
TestGameMapBaseStorage::testAccess()
{
    game::map::BaseStorage testee;

    // Initial value: everything is invalid
    TS_ASSERT(!testee.get(0).isValid());
    TS_ASSERT(!testee.get(1).isValid());
    TS_ASSERT(!testee.get(2).isValid());
    TS_ASSERT(!testee.get(3).isValid());
    TS_ASSERT(!testee.get(4).isValid());

    // Set some values
    testee.set(0, 66);
    testee.set(1, 77);
    testee.set(3, 88);

    // Read back
    TS_ASSERT(!testee.get(0).isValid());
    TS_ASSERT( testee.get(1).isValid());
    TS_ASSERT(!testee.get(2).isValid());
    TS_ASSERT( testee.get(3).isValid());
    TS_ASSERT(!testee.get(4).isValid());

    // Element access
    TS_ASSERT(testee.at(0) == 0);
    TS_ASSERT(testee.at(1) != 0);
    TS_ASSERT(testee.at(2) != 0);
    TS_ASSERT(testee.at(3) != 0);
    TS_ASSERT(testee.at(4) == 0);

    // Size access: maximum element we set is 3
    TS_ASSERT_EQUALS(testee.size(), 4);
}

/** Test isValid(). */
void
TestGameMapBaseStorage::testValid()
{
    game::map::BaseStorage testee;
    TS_ASSERT(!testee.isValid());

    testee.set(3, 7);
    TS_ASSERT(testee.isValid());

    testee.set(3, afl::base::Nothing);
    TS_ASSERT(!testee.isValid());

    testee.clear();
    TS_ASSERT(!testee.isValid());
}

/** Test clear(). */
void
TestGameMapBaseStorage::testClear()
{
    game::map::BaseStorage testee;

    // Initial value: everything is invalid
    TS_ASSERT(!testee.get(0).isValid());
    TS_ASSERT(!testee.get(1).isValid());
    TS_ASSERT(!testee.get(2).isValid());

    // Set value
    testee.set(1, 77);
    TS_ASSERT_EQUALS(testee.get(1).orElse(-1), 77);

    // Element access
    testee.clear();
    TS_ASSERT_EQUALS(testee.get(1).orElse(-1), -1);
}
