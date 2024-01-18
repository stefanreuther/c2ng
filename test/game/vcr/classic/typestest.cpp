/**
  *  \file test/game/vcr/classic/typestest.cpp
  *  \brief Test for game::vcr::classic::Types
  */

#include "game/vcr/classic/types.hpp"
#include "afl/test/testrunner.hpp"

/** This is a header file containing types.
    This test just verifies that the header file is self-contained. */
AFL_TEST("game.vcr.classic.Types", a)
{
    namespace c = game::vcr::classic;
    a.checkEqual("01", flipSide(c::LeftSide), c::RightSide);
    a.checkEqual("02", flipSide(c::RightSide), c::LeftSide);
}
