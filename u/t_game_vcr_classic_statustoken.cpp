/**
  *  \file u/t_game_vcr_classic_statustoken.cpp
  *  \brief Test for game::vcr::classic::StatusToken
  */

#include "game/vcr/classic/statustoken.hpp"

#include "t_game_vcr_classic.hpp"

/** Simple test. */
void
TestGameVcrClassicStatusToken::testIt()
{
    game::vcr::classic::StatusToken testee(123);
    TS_ASSERT_EQUALS(testee.getTime(), 123);
}

