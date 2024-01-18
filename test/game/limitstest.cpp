/**
  *  \file test/game/limitstest.cpp
  *  \brief Test for game::Limits
  */

#include "game/limits.hpp"

#include "afl/base/staticassert.hpp"
#include "afl/test/testrunner.hpp"

/** The main objective of this test is to verify that the header file is self-contained. */
AFL_TEST_NOARG("game.Limits")
{
    static_assert(game::MAX_PLAYERS != 0, "MAX_PLAYERS");
}
