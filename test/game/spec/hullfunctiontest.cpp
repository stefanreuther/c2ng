/**
  *  \file test/game/spec/hullfunctiontest.cpp
  *  \brief Test for game::spec::HullFunction
  */

#include "game/spec/hullfunction.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/spec/hull.hpp"

using game::spec::BasicHullFunction;
using game::spec::HullFunction;
using game::PlayerSet_t;
using game::MAX_PLAYERS;

/** Test basic data operations. */
AFL_TEST("game.spec.HullFunction:basics", a)
{
    // Create a HullFunction object with basicFunctionId=32
    HullFunction testee(32);

    // Verify defaults
    a.checkEqual("01. getPlayers",         testee.getPlayers(), PlayerSet_t::allUpTo(MAX_PLAYERS));
    a.checkEqual("02. getLevels",          testee.getLevels(),  game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS));
    a.checkEqual("03. getKind",            testee.getKind(),    HullFunction::AssignedToShip);
    a.checkEqual("04. getHostId",          testee.getHostId(),  -1);
    a.checkEqual("05. getBasicFunctionId", testee.getBasicFunctionId(), 32);
    a.check("06. isSame",                  testee.isSame(testee));

    // Update
    testee.setLevels(game::ExperienceLevelSet_t::allUpTo(2));
    testee.setPlayers(PlayerSet_t(7));
    testee.setKind(HullFunction::AssignedToHull);
    testee.setHostId(42);
    testee.setBasicFunctionId(12);

    // Verify update
    a.checkEqual("11. getPlayers",         testee.getPlayers(), PlayerSet_t(7));
    a.checkEqual("12. getLevels",          testee.getLevels(),  game::ExperienceLevelSet_t::allUpTo(2));
    a.checkEqual("13. getKind",            testee.getKind(),    HullFunction::AssignedToHull);
    a.checkEqual("14. getHostId",          testee.getHostId(),  42);
    a.checkEqual("15. getBasicFunctionId", testee.getBasicFunctionId(), 12);
    a.check("16. isSame",                  testee.isSame(testee));
}

/** Test comparisons. */
AFL_TEST("game.spec.HullFunction:isSame", a)
{
    // Define a hull function
    HullFunction testee(7, game::ExperienceLevelSet_t::allUpTo(2));
    testee.setHostId(12);
    testee.setKind(HullFunction::AssignedToHull);

    // Comparisons
    a.check("01. isSame", testee.isSame(testee));
    a.check("02. isSame", testee.isSame(HullFunction(7, game::ExperienceLevelSet_t::allUpTo(2))));
    a.check("03. isSame", !testee.isSame(HullFunction(7, game::ExperienceLevelSet_t::allUpTo(3))));
    a.check("04. isSame", !testee.isSame(HullFunction(9, game::ExperienceLevelSet_t::allUpTo(2))));
    a.check("05. isSame", HullFunction(7, game::ExperienceLevelSet_t::allUpTo(2)).isSame(testee));
    a.check("06. isSame", !HullFunction(7, game::ExperienceLevelSet_t::allUpTo(3)).isSame(testee));
    a.check("07. isSame", !HullFunction(9, game::ExperienceLevelSet_t::allUpTo(2)).isSame(testee));
}

/*
 *  Test getDefaultAssignment
 */

// Tow
// - one engine, no one-engine-towing
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:Tow:one-engine", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(3);
    hull.setNumEngines(1);
    config[config.AllowOneEngineTowing].set(false);
    a.check("", HullFunction::getDefaultAssignment(BasicHullFunction::Tow, config, hull).empty());
}

// - one engine, one-engine-towing enabled
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:Tow:one-engine-enabled", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(3);
    hull.setNumEngines(1);
    config[config.AllowOneEngineTowing].set(true);
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::Tow, config, hull), PlayerSet_t::allUpTo(MAX_PLAYERS));
}

// - two engines
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:Tow:two-engines", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(3);
    hull.setNumEngines(2);
    config[config.AllowOneEngineTowing].set(false);
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::Tow, config, hull), PlayerSet_t::allUpTo(MAX_PLAYERS));
}

// Boarding
// - all disabled
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:Boarding:disabled", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(7);
    config.setDefaultValues();
    config[config.AllowPrivateerTowCapture].set(false);
    config[config.AllowCrystalTowCapture].set(false);
    a.check("", HullFunction::getDefaultAssignment(BasicHullFunction::Tow, config, hull).empty());
}

// - privateer enabled
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:Boarding:privateer", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(7);
    config.setDefaultValues();
    config[config.AllowPrivateerTowCapture].set(true);
    config[config.AllowCrystalTowCapture].set(false);
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::Boarding, config, hull), PlayerSet_t(5));
}

// - all enabled
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:Boarding:privateer+tholian", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(7);
    config.setDefaultValues();
    config[config.AllowPrivateerTowCapture].set(true);
    config[config.AllowCrystalTowCapture].set(true);
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::Boarding, config, hull), PlayerSet_t() + 5 + 7);
}

// - nonstandard PlayerRace
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:Boarding:PlayerRace", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(7);
    config.setDefaultValues();
    config[config.AllowPrivateerTowCapture].set(true);
    config[config.AllowCrystalTowCapture].set(true);
    config[config.PlayerRace].set("5,2,7,4,1,2,3,5,7,5,1"); // must end in not-5-or-7 because that's the value that is used to pad the option to MAX_PLAYERS
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::Boarding, config, hull), PlayerSet_t() + 1 + 3 + 8 + 9 + 10);
}

// AntiCloakImmunity
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:AntiCloakImmunity", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(9);
    config.setDefaultValues();
    config[config.AntiCloakImmunity].set("yes,no,yes,no,yes,no");
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::AntiCloakImmunity, config, hull), PlayerSet_t() + 1 + 3 + 5);
}

// PlanetImmunity
// - default
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:PlanetImmunity:default", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(77);
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::PlanetImmunity, config, hull), PlayerSet_t() + 4 + 10);
}

// - rebels can be attacked
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:PlanetImmunity:rebel", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(77);
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(true);
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::PlanetImmunity, config, hull), PlayerSet_t() + 4);
}

// - nonstandard PlayerRace
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:PlanetImmunity:PlayerRace", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(77);
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);
    config[config.PlayerRace].set("1,4,10,2,3,5,6,10,4,9");
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::PlanetImmunity, config, hull), PlayerSet_t() + 2 + 3 + 8 + 9);
}

// FullWeaponry
// - disabled
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:FullWeaponry:disabled", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(77);
    config.setDefaultValues();
    config[config.AllowFedCombatBonus].set(false);
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::FullWeaponry, config, hull), PlayerSet_t());
}

// - enabled
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:FullWeaponry:enabled", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(77);
    config.setDefaultValues();
    config[config.AllowFedCombatBonus].set(true);
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::FullWeaponry, config, hull), PlayerSet_t(1));
}

// - nonstandard PlayerRace
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:FullWeaponry:PlayerRace", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(77);
    config.setDefaultValues();
    config[config.AllowFedCombatBonus].set(true);
    config[config.PlayerRace].set("2,1,3,1,5,1,7,8,9,10");
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::FullWeaponry, config, hull), PlayerSet_t() + 2 + 4 + 6);
}

// Other
AFL_TEST("game.spec.HullFunction:getDefaultAssignment:other", a)
{
    game::config::HostConfiguration config;
    game::spec::Hull hull(42);
    config.setDefaultValues();
    a.checkEqual("", HullFunction::getDefaultAssignment(BasicHullFunction::Bioscan, config, hull), PlayerSet_t());
}
