/**
  *  \file u/t_game_vcr_classic_types.cpp
  *  \brief Test for game::vcr::classic::Types
  */

#include "game/vcr/classic/types.hpp"

#include "t_game_vcr_classic.hpp"

/** This is a header file containing types.
    This test just verifies that the header file is self-contained. */
void
TestGameVcrClassicTypes::testIt()
{
    namespace c = game::vcr::classic;
    TS_ASSERT_EQUALS(flipSide(c::LeftSide), c::RightSide);
    TS_ASSERT_EQUALS(flipSide(c::RightSide), c::LeftSide);
}

