/**
  *  \file u/t_game_game.cpp
  *  \brief Test for game::Game
  */

#include "game/game.hpp"

#include "t_game.hpp"

/** Test smart pointers. */
void
TestGameGame::testRef()
{
    // Create a game and place in smart pointer
    afl::base::Ptr<game::Game> sp = new game::Game();
    game::Game* dp = sp.get();

    // Create smart pointer from dumb one
    {
        afl::base::Ptr<game::Game> sp2 = dp;
    }

    // If the pointers didn't work, this will access unallocated memory.
    dp->notifyListeners();
}
