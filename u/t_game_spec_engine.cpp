/**
  *  \file u/t_game_spec_engine.cpp
  *  \brief Test for game::spec::Engine
  */

#include "game/spec/engine.hpp"

#include "t_game_spec.hpp"

/** Test default values. */
void
TestGameSpecEngine::testFuelDefaults()
{
    game::spec::Engine e(4);

    // Must be valid for further tests
    TS_ASSERT_EQUALS(e.MAX_WARP, 9);

    // Default values
    int32_t ff;
    TS_ASSERT(e.getFuelFactor(-1, ff));    // not explicitly documented, but robust choice
    TS_ASSERT_EQUALS(ff, 0);

    TS_ASSERT(e.getFuelFactor(0, ff));
    TS_ASSERT_EQUALS(ff, 0);

    TS_ASSERT(e.getFuelFactor(1, ff));
    TS_ASSERT_EQUALS(ff, 0);

    TS_ASSERT(e.getFuelFactor(9, ff));
    TS_ASSERT_EQUALS(ff, 0);

    TS_ASSERT(!e.getFuelFactor(10, ff));

    // Because we have no fuel factors, max efficient warp is 9
    TS_ASSERT_EQUALS(e.getMaxEfficientWarp(), 9);
}

/** Test with initialized values. */
void
TestGameSpecEngine::testFuel()
{
    game::spec::Engine e(4);

    // Define a standard Transwarp drive
    for (int i = 1; i <= 9; ++i) {
        e.setFuelFactor(i, i*i*100);
    }

    // Verify
    int32_t ff;
    TS_ASSERT(e.getFuelFactor(0, ff));
    TS_ASSERT_EQUALS(ff, 0);

    TS_ASSERT(e.getFuelFactor(1, ff));
    TS_ASSERT_EQUALS(ff, 100);

    TS_ASSERT(e.getFuelFactor(9, ff));
    TS_ASSERT_EQUALS(ff, 8100);

    TS_ASSERT(!e.getFuelFactor(10, ff));

    TS_ASSERT_EQUALS(e.getMaxEfficientWarp(), 9);

    // Make it a Heavy Nova 6
    e.setFuelFactor(1, 100);
    e.setFuelFactor(2, 415);
    e.setFuelFactor(3, 940);
    e.setFuelFactor(4, 1700);
    e.setFuelFactor(5, 2600);
    e.setFuelFactor(6, 3733);
    e.setFuelFactor(7, 12300);
    e.setFuelFactor(8, 21450);
    e.setFuelFactor(9, 72900);
    TS_ASSERT_EQUALS(e.getMaxEfficientWarp(), 6);

    // Override
    e.setMaxEfficientWarp(8);
    TS_ASSERT_EQUALS(e.getMaxEfficientWarp(), 8);
}
