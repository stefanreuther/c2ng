/**
  *  \file u/t_game_extra.cpp
  *  \brief Test for game::Extra
  */

#include "game/extra.hpp"

#include "t_game.hpp"

/** Interface test.
    This is just a marker class without functionality.
    Test that the header file is self-contained, and the class is instantiatable. */
void
TestGameExtra::testInterface()
{
    game::Extra testee;
    (void) testee;
}

