/**
  *  \file test/game/map/shiptest.cpp
  *  \brief Test for game::map::Ship
  */

#include "game/map/ship.hpp"
#include "afl/test/testrunner.hpp"

/** Test basic initialisation.
    This catches header file errors. */
AFL_TEST("game.map.Ship:Init", a)
{
    game::map::Ship testee(99);
    a.checkEqual("01", testee.getId(), 99);
}
