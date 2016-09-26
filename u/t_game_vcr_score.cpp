/**
  *  \file u/t_game_vcr_score.cpp
  *  \brief Test for game::vcr::Score
  */

#include "game/vcr/score.hpp"

#include "t_game_vcr.hpp"

/** Simple test. */
void
TestGameVcrScore::testIt()
{
    // Test initialisation
    game::vcr::Score t;
    TS_ASSERT_EQUALS(t.getBuildMillipointsMin(), 0);
    TS_ASSERT_EQUALS(t.getBuildMillipointsMax(), 0);
    TS_ASSERT_EQUALS(t.getExperience(), 0);
    TS_ASSERT_EQUALS(t.getTonsDestroyed(), 0);

    // Add something
    t.addBuildMillipoints(111, 222);
    t.addExperience(300);
    t.addTonsDestroyed(4445);
    TS_ASSERT_EQUALS(t.getBuildMillipointsMin(), 111);
    TS_ASSERT_EQUALS(t.getBuildMillipointsMax(), 222);
    TS_ASSERT_EQUALS(t.getExperience(), 300);
    TS_ASSERT_EQUALS(t.getTonsDestroyed(), 4445);

    // Add again
    t.addBuildMillipoints(3, 4);
    t.addExperience(5);
    t.addTonsDestroyed(6);
    TS_ASSERT_EQUALS(t.getBuildMillipointsMin(), 114);
    TS_ASSERT_EQUALS(t.getBuildMillipointsMax(), 226);
    TS_ASSERT_EQUALS(t.getExperience(), 305);
    TS_ASSERT_EQUALS(t.getTonsDestroyed(), 4451);
}
