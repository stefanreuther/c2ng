/**
  *  \file test/game/vcr/classic/statustokentest.cpp
  *  \brief Test for game::vcr::classic::StatusToken
  */

#include "game/vcr/classic/statustoken.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("game.vcr.classic.StatusToken", a)
{
    game::vcr::classic::StatusToken testee(123);
    a.checkEqual("01. getTime", testee.getTime(), 123);
}
