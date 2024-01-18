/**
  *  \file test/game/extratest.cpp
  *  \brief Test for game::Extra
  */

#include "game/extra.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test.
    This is just a marker class without functionality.
    Test that the header file is self-contained, and the class is instantiatable. */
AFL_TEST_NOARG("game.Extra")
{
    game::Extra testee;
    (void) testee;
}
