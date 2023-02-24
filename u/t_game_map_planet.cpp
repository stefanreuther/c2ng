/**
  *  \file u/t_game_map_planet.cpp
  *  \brief Test for game::map::Planet
  */

#include "game/map/planet.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"

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

/** Test isKnownToHaveNatives() status. */
void
TestGameMapPlanet::testKnownToHaveNatives()
{
    game::map::Planet t(19);
    t.setPosition(game::map::Point(1000, 1000));
    t.setKnownToHaveNatives(true);

    afl::string::NullTranslator tx;
    afl::sys::Log log;
    t.internalCheck(game::map::Configuration(), game::PlayerSet_t(), 10, tx, log);

    TS_ASSERT(t.isKnownToHaveNatives());
    TS_ASSERT(t.hasAnyPlanetData());
}

