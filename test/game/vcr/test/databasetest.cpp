/**
  *  \file test/game/vcr/test/databasetest.cpp
  *  \brief Test for game::vcr::test::Database
  */

#include "game/vcr/test/database.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("game.vcr.test.Database", a)
{
    // Initial status
    game::vcr::test::Database testee;
    a.checkEqual("01. getNumBattles", testee.getNumBattles(), 0U);
    a.checkNull("02. getBattle", testee.getBattle(0));

    // Add some battles
    game::vcr::test::Battle& b = testee.addBattle();
    for (int i = 0; i < 30; ++i) {
        testee.addBattle();
    }

    // Verify; in particular, should not re-allocate
    a.checkEqual("11. getNumBattles", testee.getNumBattles(), 31U);
    a.checkEqual("12. getBattle", testee.getBattle(0), &b);
}
