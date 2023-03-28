/**
  *  \file u/t_game_vcr_test_database.cpp
  *  \brief Test for game::vcr::test::Database
  */

#include "game/vcr/test/database.hpp"

#include "t_game_vcr_test.hpp"

/** Simple test. */
void
TestGameVcrTestDatabase::testIt()
{
    // Initial status
    game::vcr::test::Database testee;
    TS_ASSERT_EQUALS(testee.getNumBattles(), 0U);
    TS_ASSERT(testee.getBattle(0) == 0);

    // Add some battles
    game::vcr::test::Battle& b = testee.addBattle();
    for (int i = 0; i < 30; ++i) {
        testee.addBattle();
    }

    // Verify; in particular, should not re-allocate
    TS_ASSERT_EQUALS(testee.getNumBattles(), 31U);
    TS_ASSERT_EQUALS(testee.getBattle(0), &b);
}

