/**
  *  \file test/game/maint/difficultyratertest.cpp
  *  \brief Test for game::maint::DifficultyRater
  */

#include "game/maint/difficultyrater.hpp"
#include "afl/test/testrunner.hpp"

// Default-constructed
AFL_TEST("game.maint.DifficultyRater:default", a)
{
    game::maint::DifficultyRater testee;
    a.checkEqual("ShiplistRating",   testee.isRatingKnown(game::maint::DifficultyRater::ShiplistRating), false);
    a.checkEqual("MineralRating",    testee.isRatingKnown(game::maint::DifficultyRater::MineralRating), false);
    a.checkEqual("NativeRating",     testee.isRatingKnown(game::maint::DifficultyRater::NativeRating), false);
    a.checkEqual("ProductionRating", testee.isRatingKnown(game::maint::DifficultyRater::ProductionRating), false);
    a.checkEqual("getTotalRating",   testee.getTotalRating(), 1.0);
}

// Some config
AFL_TEST("game.maint.DifficultyRater:config", a)
{
    game::maint::DifficultyRater testee;
    testee.addConfigurationValue("amaster.PlanetCoreRangesUsual", "2000,2000,2000,2000,10000,10000,10000,10000");
    testee.addConfigurationValue("amaster.PlanetCoreUsualFrequency", "100");
    testee.addConfigurationValue("amaster.PlanetCoreRangesAlternate", "0,0,0,0,0,0,0,0");
    testee.addConfigurationValue("amaster.PlanetSurfaceRanges", "1000,1000,1000,1000,2000,2000,2000,2000");
    // produces average per planet = 3*(1500 + 6000) = 22500,
    // yielding a difficulty of (1800/22500)^0.33 = 0.4345

    a.checkEqual("ShiplistRating",   testee.isRatingKnown(game::maint::DifficultyRater::ShiplistRating), false);
    a.checkEqual("MineralRating",    testee.isRatingKnown(game::maint::DifficultyRater::MineralRating), true);
    a.checkEqual("NativeRating",     testee.isRatingKnown(game::maint::DifficultyRater::NativeRating), false);
    a.checkEqual("ProductionRating", testee.isRatingKnown(game::maint::DifficultyRater::ProductionRating), false);
    a.checkEqual("getTotalRating",   int(10000*testee.getTotalRating()), 4345);
}
