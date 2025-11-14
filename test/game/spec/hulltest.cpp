/**
  *  \file test/game/spec/hulltest.cpp
  *  \brief Test for game::spec::Hull
  */

#include "game/spec/hull.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"

using afl::base::Ref;
using game::config::HostConfiguration;

/** Accessor tests. */
AFL_TEST("game.spec.Hull:basics", a)
{
    game::spec::Hull h(7);

    // Initial state
    a.checkEqual("01. getExternalPictureNumber", h.getExternalPictureNumber(), 0);
    a.checkEqual("02. getInternalPictureNumber", h.getInternalPictureNumber(), 0);
    a.checkEqual("03. getMaxFuel",               h.getMaxFuel(), 0);
    a.checkEqual("04. getMaxCrew",               h.getMaxCrew(), 0);
    a.checkEqual("05. getNumEngines",            h.getNumEngines(), 0);
    a.checkEqual("06. getMaxCargo",              h.getMaxCargo(), 0);
    a.checkEqual("07. getNumBays",               h.getNumBays(), 0);
    a.checkEqual("08. getMaxLaunchers",          h.getMaxLaunchers(), 0);
    a.checkEqual("09. getMaxBeams",              h.getMaxBeams(), 0);
    a.checkEqual("10. getId",                    h.getId(), 7);

    // Configure
    h.setExternalPictureNumber(230);
    h.setInternalPictureNumber(333);
    h.setMaxFuel(600);
    h.setMaxCrew(1200);
    h.setNumEngines(3);
    h.setMaxCargo(2400);
    h.setNumBays(4);
    h.setMaxLaunchers(2);
    h.setMaxBeams(12);

    // Verify
    a.checkEqual("11. getExternalPictureNumber", h.getExternalPictureNumber(), 230);
    a.checkEqual("12. getInternalPictureNumber", h.getInternalPictureNumber(), 333);
    a.checkEqual("13. getMaxFuel",               h.getMaxFuel(), 600);
    a.checkEqual("14. getMaxCrew",               h.getMaxCrew(), 1200);
    a.checkEqual("15. getNumEngines",            h.getNumEngines(), 3);
    a.checkEqual("16. getMaxCargo",              h.getMaxCargo(), 2400);
    a.checkEqual("17. getNumBays",               h.getNumBays(), 4);
    a.checkEqual("18. getMaxLaunchers",          h.getMaxLaunchers(), 2);
    a.checkEqual("19. getMaxBeams",              h.getMaxBeams(), 12);
    a.checkEqual("20. getId",                    h.getId(), 7);
}

/** Test hull functions. */
AFL_TEST("game.spec.Hull:getHullFunctions", a)
{
    game::spec::Hull h(88);
    const game::spec::Hull& ch(h);

    // General access
    a.checkEqual("01", &h.getHullFunctions(true), &ch.getHullFunctions(true));
    a.checkEqual("02", &h.getHullFunctions(false), &ch.getHullFunctions(false));
    a.checkDifferent("03", &h.getHullFunctions(true), &h.getHullFunctions(false));

    // Functionality litmus test
    const game::spec::ModifiedHullFunctionList::Function_t fn(game::spec::ModifiedHullFunctionList::Function_t(333));

    h.changeHullFunction(fn, game::PlayerSet_t(1), game::PlayerSet_t(), true);
    a.checkNonNull("11", h.getHullFunctions(true).findEntry(fn));
    a.checkEqual("12", ch.getHullFunctions(true).findEntry(fn), h.getHullFunctions(true).findEntry(fn));
    a.checkNull("13", h.getHullFunctions(false).findEntry(fn));

    h.clearHullFunctions();
    a.checkNull("21", h.getHullFunctions(true).findEntry(fn));
    a.checkNull("22", ch.getHullFunctions(true).findEntry(fn));
}

/** Test getTurnFuelUsage(). */
AFL_TEST("game.spec.Hull:getTurnFuelUsage", a)
{
    // Values verified using c2hosttest/ship/02_fuelperturn
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[HostConfiguration::FuelUsagePerTurnFor100KT].set(5);

    game::spec::Hull t(1);

    // Outrider (75 kt) will burn 4 kt
    t.setMass(75);
    a.checkEqual("01", t.getTurnFuelUsage(1, false, config), 4);

    // Banshee (120 kt) will burn 6 kt
    t.setMass(120);
    a.checkEqual("11", t.getTurnFuelUsage(1, false, config), 6);

    // Loki (101 kt) will burn 6 kt
    t.setMass(101);
    a.checkEqual("21", t.getTurnFuelUsage(1, false, config), 6);

    // NFC (10 kt) will burn 1 kt
    t.setMass(10);
    a.checkEqual("31", t.getTurnFuelUsage(1, false, config), 1);

    // Dark Wing (491 kt) will burn 25 kt
    t.setMass(491);
    a.checkEqual("41", t.getTurnFuelUsage(1, false, config), 25);
}

/** Test getCloakFuelUsage. */
AFL_TEST("game.spec.Hull:getCloakFuelUsage", a)
{
    // Values verified using c2hosttest/ship/02_fuelperturn
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[HostConfiguration::CloakFuelBurn].set(5);

    game::spec::Hull t(1);

    // BR4 (55 kt) will burn 5 kt
    t.setMass(55);
    a.checkEqual("01", t.getCloakFuelUsage(1, config), 5);

    // LCC (160 kt) will burn 8 kt
    t.setMass(160);
    a.checkEqual("11", t.getCloakFuelUsage(1, config), 8);

    // Death Specula (113 kt) will burn 5 kt
    t.setMass(113);
    a.checkEqual("21", t.getCloakFuelUsage(1, config), 5);
}

/** Test getMineHitDamage. */
AFL_TEST("game.spec.Hull:getMineHitDamage", a)
{
    // Values verified using c2hosttest/mine/02_damage
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    game::HostVersion h(game::HostVersion::Host, MKVERSION(3,22,40));
    game::HostVersion p(game::HostVersion::PHost, MKVERSION(4,0,0));

    game::spec::Hull t(3);

    // T-Rex (#23), 421 kt -> 24% damage in THost, 23% damage in PHost
    t.setMass(421);
    a.checkEqual("01", t.getMineHitDamage(1, false, h, config), 24);
    a.checkEqual("02", t.getMineHitDamage(1, false, p, config), 23);

    // Banshee (#6), 120 kt -> 83% damage in either host
    t.setMass(120);
    a.checkEqual("11", t.getMineHitDamage(1, false, h, config), 83);
    a.checkEqual("12", t.getMineHitDamage(1, false, p, config), 83);

    // Bohemian on Webs (#3), 32 kt -> 30% damage in THost, 31% damage in PHost
    t.setMass(32);
    a.checkEqual("21", t.getMineHitDamage(1, true, h, config), 30);
    a.checkEqual("22", t.getMineHitDamage(1, true, p, config), 31);
}

/** Test point computations. */
AFL_TEST("game.spec.Hull:points", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    game::HostVersion h(game::HostVersion::Host, MKVERSION(3,22,40));
    game::HostVersion p(game::HostVersion::PHost, MKVERSION(4,0,0));

    game::spec::Hull t(77);

    // Vendetta/Dwarfstar (100 kt)
    t.setMass(100);
    a.checkEqual("01. getPointsToBuild", t.getPointsToBuild(1, h, config), 2);
    a.checkEqual("02. getPointsToBuild", t.getPointsToBuild(1, p, config), 400); // minimum cost
    a.checkEqual("03. getPointsForKilling", t.getPointsForKilling(1, h, config), 2);
    a.checkEqual("04. getPointsForKilling", t.getPointsForKilling(1, p, config), 120);
    a.checkEqual("05. getPointsForScrapping", t.getPointsForScrapping(1, h, config), 1);
    a.checkEqual("06. getPointsForScrapping", t.getPointsForScrapping(1, p, config), 40);

    // Loki (101 kt)
    t.setMass(101);
    a.checkEqual("11. getPointsToBuild", t.getPointsToBuild(1, h, config), 3);
    a.checkEqual("12. getPointsToBuild", t.getPointsToBuild(1, p, config), 400); // minimum cost
}
