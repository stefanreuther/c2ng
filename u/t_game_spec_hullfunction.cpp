/**
  *  \file u/t_game_spec_hullfunction.cpp
  *  \brief Test for game::spec::HullFunction
  */

#include "game/spec/hullfunction.hpp"

#include "t_game_spec.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/basichullfunction.hpp"

/** Test basic data operations. */
void
TestGameSpecHullFunction::testIt()
{
    // Create a HullFunction object with basicFunctionId=32
    game::spec::HullFunction testee(32);

    // Verify defaults
    TS_ASSERT_EQUALS(testee.getPlayers(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    TS_ASSERT_EQUALS(testee.getLevels(),  game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS));
    TS_ASSERT_EQUALS(testee.getKind(),    game::spec::HullFunction::AssignedToShip);
    TS_ASSERT_EQUALS(testee.getHostId(),  -1);
    TS_ASSERT_EQUALS(testee.getBasicFunctionId(), 32);
    TS_ASSERT(testee.isSame(testee));

    // Update
    testee.setLevels(game::ExperienceLevelSet_t::allUpTo(2));
    testee.setPlayers(game::PlayerSet_t(7));
    testee.setKind(game::spec::HullFunction::AssignedToHull);
    testee.setHostId(42);
    testee.setBasicFunctionId(12);

    // Verify update
    TS_ASSERT_EQUALS(testee.getPlayers(), game::PlayerSet_t(7));
    TS_ASSERT_EQUALS(testee.getLevels(),  game::ExperienceLevelSet_t::allUpTo(2));
    TS_ASSERT_EQUALS(testee.getKind(),    game::spec::HullFunction::AssignedToHull);
    TS_ASSERT_EQUALS(testee.getHostId(),  42);
    TS_ASSERT_EQUALS(testee.getBasicFunctionId(), 12);
    TS_ASSERT(testee.isSame(testee));
}

/** Test comparisons. */
void
TestGameSpecHullFunction::testCompare()
{
    // Define a hull function
    game::spec::HullFunction testee(7, game::ExperienceLevelSet_t::allUpTo(2));
    testee.setHostId(12);
    testee.setKind(game::spec::HullFunction::AssignedToHull);

    // Comparisons
    TS_ASSERT(testee.isSame(testee));
    TS_ASSERT(testee.isSame(game::spec::HullFunction(7, game::ExperienceLevelSet_t::allUpTo(2))));
    TS_ASSERT(!testee.isSame(game::spec::HullFunction(7, game::ExperienceLevelSet_t::allUpTo(3))));
    TS_ASSERT(!testee.isSame(game::spec::HullFunction(9, game::ExperienceLevelSet_t::allUpTo(2))));
    TS_ASSERT(game::spec::HullFunction(7, game::ExperienceLevelSet_t::allUpTo(2)).isSame(testee));
    TS_ASSERT(!game::spec::HullFunction(7, game::ExperienceLevelSet_t::allUpTo(3)).isSame(testee));
    TS_ASSERT(!game::spec::HullFunction(9, game::ExperienceLevelSet_t::allUpTo(2)).isSame(testee));
}

/** Test getDefaultAssignment(). */
void
TestGameSpecHullFunction::testGetDefault()
{
    using game::spec::BasicHullFunction;
    using game::spec::HullFunction;
    using game::PlayerSet_t;
    using game::MAX_PLAYERS;

    // Tow
    // - one engine, no one-engine-towing
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(3);
        hull.setNumEngines(1);
        config[config.AllowOneEngineTowing].set(false);
        TS_ASSERT(HullFunction::getDefaultAssignment(BasicHullFunction::Tow, config, hull).empty());
    }
    // - one engine, one-engine-towing enabled
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(3);
        hull.setNumEngines(1);
        config[config.AllowOneEngineTowing].set(true);
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::Tow, config, hull), PlayerSet_t::allUpTo(MAX_PLAYERS));
    }
    // - two engines
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(3);
        hull.setNumEngines(2);
        config[config.AllowOneEngineTowing].set(false);
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::Tow, config, hull), PlayerSet_t::allUpTo(MAX_PLAYERS));
    }

    // Boarding
    // - all disabled
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(7);
        config.setDefaultValues();
        config[config.AllowPrivateerTowCapture].set(false);
        config[config.AllowCrystalTowCapture].set(false);
        TS_ASSERT(HullFunction::getDefaultAssignment(BasicHullFunction::Tow, config, hull).empty());
    }
    // - privateer enabled
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(7);
        config.setDefaultValues();
        config[config.AllowPrivateerTowCapture].set(true);
        config[config.AllowCrystalTowCapture].set(false);
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::Boarding, config, hull), PlayerSet_t(5));
    }
    // - all enabled
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(7);
        config.setDefaultValues();
        config[config.AllowPrivateerTowCapture].set(true);
        config[config.AllowCrystalTowCapture].set(true);
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::Boarding, config, hull), PlayerSet_t() + 5 + 7);
    }
    // - nonstandard PlayerRace
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(7);
        config.setDefaultValues();
        config[config.AllowPrivateerTowCapture].set(true);
        config[config.AllowCrystalTowCapture].set(true);
        config[config.PlayerRace].set("5,2,7,4,1,2,3,5,7,5,1"); // must end in not-5-or-7 because that's the value that is used to pad the option to MAX_PLAYERS
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::Boarding, config, hull), PlayerSet_t() + 1 + 3 + 8 + 9 + 10);
    }

    // AntiCloakImmunity
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(9);
        config.setDefaultValues();
        config[config.AntiCloakImmunity].set("yes,no,yes,no,yes,no");
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::AntiCloakImmunity, config, hull), PlayerSet_t() + 1 + 3 + 5);
    }

    // PlanetImmunity
    // - default
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(77);
        config.setDefaultValues();
        config[config.PlanetsAttackKlingons].set(false);
        config[config.PlanetsAttackRebels].set(false);
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::PlanetImmunity, config, hull), PlayerSet_t() + 4 + 10);
    }
    // - rebels can be attacked
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(77);
        config.setDefaultValues();
        config[config.PlanetsAttackKlingons].set(false);
        config[config.PlanetsAttackRebels].set(true);
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::PlanetImmunity, config, hull), PlayerSet_t() + 4);
    }
    // - nonstandard PlayerRace
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(77);
        config.setDefaultValues();
        config[config.PlanetsAttackKlingons].set(false);
        config[config.PlanetsAttackRebels].set(false);
        config[config.PlayerRace].set("1,4,10,2,3,5,6,10,4,9");
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::PlanetImmunity, config, hull), PlayerSet_t() + 2 + 3 + 8 + 9);
    }

    // FullWeaponry
    // - disabled
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(77);
        config.setDefaultValues();
        config[config.AllowFedCombatBonus].set(false);
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::FullWeaponry, config, hull), PlayerSet_t());
    }
    // - enabled
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(77);
        config.setDefaultValues();
        config[config.AllowFedCombatBonus].set(true);
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::FullWeaponry, config, hull), PlayerSet_t(1));
    }
    // - nonstandard PlayerRace
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(77);
        config.setDefaultValues();
        config[config.AllowFedCombatBonus].set(true);
        config[config.PlayerRace].set("2,1,3,1,5,1,7,8,9,10");
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::FullWeaponry, config, hull), PlayerSet_t() + 2 + 4 + 6);
    }

    // Other
    {
        game::config::HostConfiguration config;
        game::spec::Hull hull(42);
        config.setDefaultValues();
        TS_ASSERT_EQUALS(HullFunction::getDefaultAssignment(BasicHullFunction::Bioscan, config, hull), PlayerSet_t());
    }
}
