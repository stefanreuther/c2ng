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
    TS_ASSERT_EQUALS(t.getBuildMillipoints().min(), 0);
    TS_ASSERT_EQUALS(t.getBuildMillipoints().max(), 0);
    TS_ASSERT_EQUALS(t.getExperience().min(), 0);
    TS_ASSERT_EQUALS(t.getExperience().max(), 0);
    TS_ASSERT_EQUALS(t.getTonsDestroyed().min(), 0);
    TS_ASSERT_EQUALS(t.getTonsDestroyed().max(), 0);

    // Add something
    t.addBuildMillipoints(game::vcr::Score::Range_t(111, 222));
    t.addExperience(game::vcr::Score::Range_t(290, 300));
    t.addTonsDestroyed(game::vcr::Score::Range_t(4444, 4445));
    TS_ASSERT_EQUALS(t.getBuildMillipoints().min(), 111);
    TS_ASSERT_EQUALS(t.getBuildMillipoints().max(), 222);
    TS_ASSERT_EQUALS(t.getExperience().min(), 290);
    TS_ASSERT_EQUALS(t.getExperience().max(), 300);
    TS_ASSERT_EQUALS(t.getTonsDestroyed().min(), 4444);
    TS_ASSERT_EQUALS(t.getTonsDestroyed().max(), 4445);

    // Add again
    t.addBuildMillipoints(game::vcr::Score::Range_t(3, 4));
    t.addExperience(game::vcr::Score::Range_t::fromValue(5));
    t.addTonsDestroyed(game::vcr::Score::Range_t::fromValue(6));
    TS_ASSERT_EQUALS(t.getBuildMillipoints().min(), 114);
    TS_ASSERT_EQUALS(t.getBuildMillipoints().max(), 226);
    TS_ASSERT_EQUALS(t.getExperience().min(), 295);
    TS_ASSERT_EQUALS(t.getExperience().max(), 305);
    TS_ASSERT_EQUALS(t.getTonsDestroyed().min(), 4450);
    TS_ASSERT_EQUALS(t.getTonsDestroyed().max(), 4451);
}
