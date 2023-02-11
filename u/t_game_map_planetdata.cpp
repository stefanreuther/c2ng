/**
  *  \file u/t_game_map_planetdata.cpp
  *  \brief Test for game::map::PlanetData
  */

#include "game/map/planetdata.hpp"

#include "t_game_map.hpp"

/** Mostly, test that the header-file is well-formed. */
void
TestGameMapPlanetData::testIt()
{
    // We can default-construct.
    game::map::PlanetData a;

    // We can construct from an Id.
    game::map::PlanetData b(10);
}

