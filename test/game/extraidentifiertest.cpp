/**
  *  \file test/game/extraidentifiertest.cpp
  *  \brief Test for game::ExtraIdentifier
  */

#include "game/extraidentifier.hpp"

#include "afl/test/testrunner.hpp"
#include "game/extra.hpp"

namespace {
    class MyExtra : public game::Extra { };
}

/** Simple test. */
AFL_TEST_NOARG("game.ExtraIdentifier")
{
    // The whole point of ExtraIdentifier is to create static instances.
    game::ExtraIdentifier<int,MyExtra> testee;
    (void) testee;
}
