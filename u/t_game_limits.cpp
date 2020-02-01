/**
  *  \file u/t_game_limits.cpp
  *  \brief Test for game::Limits
  */

#include "game/limits.hpp"

#include "t_game.hpp"
#include "afl/base/staticassert.hpp"

/** The main objective of this test is to verify that the header file is self-contained. */
void
TestGameLimits::testIt()
{
    static_assert(game::MAX_PLAYERS != 0, "MAX_PLAYERS");
}

