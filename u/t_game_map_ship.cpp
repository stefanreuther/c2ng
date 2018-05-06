/**
  *  \file u/t_game_map_ship.cpp
  *  \brief Test for game::map::Ship
  */

#include "game/map/ship.hpp"

#include "t_game_map.hpp"

/** Test basic initialisation.
    This catches header file errors. */
void
TestGameMapShip::testInit()
{
    game::map::Ship testee(99);
    TS_ASSERT_EQUALS(testee.getId(), 99);
}

