/**
  *  \file test/game/map/planetdatatest.cpp
  *  \brief Test for game::map::PlanetData
  */

#include "game/map/planetdata.hpp"
#include "afl/test/testrunner.hpp"

/** Mostly, test that the header-file is well-formed. */
AFL_TEST_NOARG("game.map.PlanetData")
{
    // We can default-construct.
    game::map::PlanetData a;

    // We can construct from an Id.
    game::map::PlanetData b(10);
}
