/**
  *  \file u/t_game_actions_preconditions.cpp
  *  \brief Test for game::actions::Preconditions
  */

#include "game/actions/preconditions.hpp"

#include "t_game_actions.hpp"
#include "game/map/ship.hpp"
#include "game/map/planet.hpp"
#include "game/exception.hpp"
#include "game/map/configuration.hpp"
#include "afl/string/nulltranslator.hpp"

namespace {
    void addBase(game::map::Planet& planet)
    {
        game::map::BaseData data;
        data.owner = 1;
        data.numBaseDefensePosts = 9;
        data.damage = 0;
        planet.addCurrentBaseData(data, game::PlayerSet_t(1));
        planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(1));

        game::map::Configuration config;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        planet.internalCheck(config, tx, log);
    }
}

/** Test ship. */
void
TestGameActionsPreconditions::testShip()
{
    // Uninitialized object throws
    game::map::Ship ship(42);
    TS_ASSERT_THROWS(game::actions::mustBePlayed(ship), game::Exception);

    // ReadOnly is not sufficient
    ship.setPlayability(ship.ReadOnly);
    TS_ASSERT_THROWS(game::actions::mustBePlayed(ship), game::Exception);

    // Playable is sufficient
    ship.setPlayability(ship.Playable);
    TS_ASSERT_THROWS_NOTHING(game::actions::mustBePlayed(ship));
}

/** Test planet. */
void
TestGameActionsPreconditions::testPlanet()
{
    // Uninitialized object throws
    game::map::Planet planet(42);
    TS_ASSERT_THROWS(game::actions::mustBePlayed(planet), game::Exception);

    // ReadOnly is not sufficient
    planet.setPlayability(planet.ReadOnly);
    TS_ASSERT_THROWS(game::actions::mustBePlayed(planet), game::Exception);

    // Playable is sufficient
    planet.setPlayability(planet.Playable);
    TS_ASSERT_THROWS_NOTHING(game::actions::mustBePlayed(planet));
}

/** Test base. */
void
TestGameActionsPreconditions::testBase()
{
    {
        // Uninitialized object throws
        game::map::Planet planet(42);
        TS_ASSERT_THROWS(game::actions::mustHavePlayedBase(planet), game::Exception);

        // Give it a base. Still not sufficient
        addBase(planet);
        TS_ASSERT_THROWS(game::actions::mustHavePlayedBase(planet), game::Exception);

        // ReadOnly is not sufficient
        planet.setPlayability(planet.ReadOnly);
        TS_ASSERT_THROWS(game::actions::mustHavePlayedBase(planet), game::Exception);

        // Playable is sufficient
        planet.setPlayability(planet.Playable);
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHavePlayedBase(planet));
    }

    {
        // Playable planet throws if it has no base
        game::map::Planet planet(42);
        planet.setPlayability(planet.Playable);
        TS_ASSERT_THROWS(game::actions::mustHavePlayedBase(planet), game::Exception);

        // Add base
        addBase(planet);
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHavePlayedBase(planet));
    }
}
