/**
  *  \file test/game/spec/enginetest.cpp
  *  \brief Test for game::spec::Engine
  */

#include "game/spec/engine.hpp"
#include "afl/test/testrunner.hpp"

/** Test default values. */
AFL_TEST("game.spec.Engine:fuel:defaults", a)
{
    game::spec::Engine e(4);

    // Must be valid for further tests
    a.checkEqual("01. MAX_WARP", e.MAX_WARP, 9);

    // Default values
    int32_t ff;
    a.check("11. getFuelFactor", e.getFuelFactor(-1, ff));    // not explicitly documented, but robust choice
    a.checkEqual("12. fuel factor", ff, 0);

    a.check("21. getFuelFactor", e.getFuelFactor(0, ff));
    a.checkEqual("22. fuel factor", ff, 0);

    a.check("31. getFuelFactor", e.getFuelFactor(1, ff));
    a.checkEqual("32. fuel factor", ff, 0);

    a.check("41. getFuelFactor", e.getFuelFactor(9, ff));
    a.checkEqual("42. fuel factor", ff, 0);

    a.check("51. getFuelFactor", !e.getFuelFactor(10, ff));

    // Because we have no fuel factors, max efficient warp is 9
    a.checkEqual("61. getMaxEfficientWarp", e.getMaxEfficientWarp(), 9);
}

/** Test with initialized values. */
AFL_TEST("game.spec.Engine:fuel:values", a)
{
    game::spec::Engine e(4);

    // Define a standard Transwarp drive
    for (int i = 1; i <= 9; ++i) {
        e.setFuelFactor(i, i*i*100);
    }

    // Verify
    int32_t ff;
    a.check("01. getFuelFactor", e.getFuelFactor(0, ff));
    a.checkEqual("02. fuel factor", ff, 0);

    a.check("11. getFuelFactor", e.getFuelFactor(1, ff));
    a.checkEqual("12. fuel factor", ff, 100);

    a.check("21. getFuelFactor", e.getFuelFactor(9, ff));
    a.checkEqual("22. fuel factor", ff, 8100);

    a.check("31", !e.getFuelFactor(10, ff));

    a.checkEqual("41. getMaxEfficientWarp", e.getMaxEfficientWarp(), 9);

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
    a.checkEqual("51. getMaxEfficientWarp", e.getMaxEfficientWarp(), 6);

    // Override
    e.setMaxEfficientWarp(8);
    a.checkEqual("61. getMaxEfficientWarp", e.getMaxEfficientWarp(), 8);
}
