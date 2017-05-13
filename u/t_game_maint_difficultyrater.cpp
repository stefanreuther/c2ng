/**
  *  \file u/t_game_maint_difficultyrater.cpp
  *  \brief Test for game::maint::DifficultyRater
  */

#include "game/maint/difficultyrater.hpp"

#include "t_game_maint.hpp"

/** Simple tests. */
void
TestGameMaintDifficultyRater::testSimple()
{
    // Default-constructed
    {
        game::maint::DifficultyRater testee;
        TS_ASSERT_EQUALS(testee.isRatingKnown(game::maint::DifficultyRater::ShiplistRating), false);
        TS_ASSERT_EQUALS(testee.isRatingKnown(game::maint::DifficultyRater::MineralRating), false);
        TS_ASSERT_EQUALS(testee.isRatingKnown(game::maint::DifficultyRater::NativeRating), false);
        TS_ASSERT_EQUALS(testee.isRatingKnown(game::maint::DifficultyRater::ProductionRating), false);
        TS_ASSERT_EQUALS(testee.getTotalRating(), 1.0);
    }

    // Some config
    {
        game::maint::DifficultyRater testee;
        testee.addConfigurationValue("amaster.PlanetCoreRangesUsual", "2000,2000,2000,2000,10000,10000,10000,10000");
        testee.addConfigurationValue("amaster.PlanetCoreUsualFrequency", "100");
        testee.addConfigurationValue("amaster.PlanetCoreRangesAlternate", "0,0,0,0,0,0,0,0");
        testee.addConfigurationValue("amaster.PlanetSurfaceRanges", "1000,1000,1000,1000,2000,2000,2000,2000");
        // produces average per planet = 3*(1500 + 6000) = 22500,
        // yielding a difficulty of (1800/22500)^0.33 = 0.4345

        TS_ASSERT_EQUALS(testee.isRatingKnown(game::maint::DifficultyRater::ShiplistRating), false);
        TS_ASSERT_EQUALS(testee.isRatingKnown(game::maint::DifficultyRater::MineralRating), true);
        TS_ASSERT_EQUALS(testee.isRatingKnown(game::maint::DifficultyRater::NativeRating), false);
        TS_ASSERT_EQUALS(testee.isRatingKnown(game::maint::DifficultyRater::ProductionRating), false);
        TS_ASSERT_EQUALS(int(10000*testee.getTotalRating()), 4345);
    }
}
