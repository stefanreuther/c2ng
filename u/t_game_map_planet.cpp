/**
  *  \file u/t_game_map_planet.cpp
  *  \brief Test for game::map::Planet
  */

#include "game/map/planet.hpp"

#include "t_game_map.hpp"

/** Test AutobuildSettings object. */
void
TestGameMapPlanet::testAutobuildSettings()
{
    game::map::Planet::AutobuildSettings t;

    // Needs to be properly default-initialized to "unknown"
    TS_ASSERT_EQUALS(t.goal[0].isValid(), false);
    TS_ASSERT_EQUALS(t.speed[0].isValid(), false);
}

/** Test copying. */
void
TestGameMapPlanet::testCopy()
{
    game::map::Planet t(19);
    t.setPlayability(game::map::Planet::Playable);

    game::map::Planet t2(t);
    TS_ASSERT_EQUALS(t2.getPlayability(), game::map::Planet::Playable);
}

