/**
  *  \file test/game/vcr/classic/nullvisualizertest.cpp
  *  \brief Test for game::vcr::classic::NullVisualizer
  */

#include "game/vcr/classic/nullvisualizer.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test.
    Tests that this class is complete. */
AFL_TEST_NOARG("game.vcr.classic.NullVisualizer")
{
    game::vcr::classic::NullVisualizer testee;
}
