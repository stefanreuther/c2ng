/**
  *  \file test/game/vcr/flak/nullvisualizertest.cpp
  *  \brief Test for game::vcr::flak::NullVisualizer
  */

#include "game/vcr/flak/nullvisualizer.hpp"

#include "afl/test/testrunner.hpp"

AFL_TEST_NOARG("game.vcr.flak.NullVisualizer")
{
    // This test basically verifies only that NullVisualizer is a complete class.
    game::vcr::flak::NullVisualizer testee;
}
