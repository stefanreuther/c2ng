/**
  *  \file u/t_game_spec_hull.cpp
  *  \brief Test for game::spec::Hull
  */

#include "game/spec/hull.hpp"

#include "t_game_spec.hpp"
#include "game/config/hostconfiguration.hpp"

/** Accessor tests. */
void
TestGameSpecHull::testIt()
{
    game::spec::Hull h(7);

    // Initial state
    TS_ASSERT_EQUALS(h.getExternalPictureNumber(), 0);
    TS_ASSERT_EQUALS(h.getInternalPictureNumber(), 0);
    TS_ASSERT_EQUALS(h.getMaxFuel(), 0);
    TS_ASSERT_EQUALS(h.getMaxCrew(), 0);
    TS_ASSERT_EQUALS(h.getNumEngines(), 0);
    TS_ASSERT_EQUALS(h.getMaxCargo(), 0);
    TS_ASSERT_EQUALS(h.getNumBays(), 0);
    TS_ASSERT_EQUALS(h.getMaxLaunchers(), 0);
    TS_ASSERT_EQUALS(h.getMaxBeams(), 0);
    TS_ASSERT_EQUALS(h.getId(), 7);

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
    TS_ASSERT_EQUALS(h.getExternalPictureNumber(), 230);
    TS_ASSERT_EQUALS(h.getInternalPictureNumber(), 333);
    TS_ASSERT_EQUALS(h.getMaxFuel(), 600);
    TS_ASSERT_EQUALS(h.getMaxCrew(), 1200);
    TS_ASSERT_EQUALS(h.getNumEngines(), 3);
    TS_ASSERT_EQUALS(h.getMaxCargo(), 2400);
    TS_ASSERT_EQUALS(h.getNumBays(), 4);
    TS_ASSERT_EQUALS(h.getMaxLaunchers(), 2);
    TS_ASSERT_EQUALS(h.getMaxBeams(), 12);
    TS_ASSERT_EQUALS(h.getId(), 7);
}

/** Test hull functions. */
void
TestGameSpecHull::testHullFunctions()
{
    game::spec::Hull h(88);
    const game::spec::Hull& ch(h);

    // General access
    TS_ASSERT_EQUALS(&h.getHullFunctions(true), &ch.getHullFunctions(true));
    TS_ASSERT_EQUALS(&h.getHullFunctions(false), &ch.getHullFunctions(false));
    TS_ASSERT_DIFFERS(&h.getHullFunctions(true), &h.getHullFunctions(false));

    // Functionality litmus test
    const game::spec::ModifiedHullFunctionList::Function_t fn(game::spec::ModifiedHullFunctionList::Function_t(333));
    
    h.changeHullFunction(fn, game::PlayerSet_t(1), game::PlayerSet_t(), true);
    TS_ASSERT(h.getHullFunctions(true).findEntry(fn) != 0);
    TS_ASSERT_EQUALS(ch.getHullFunctions(true).findEntry(fn), h.getHullFunctions(true).findEntry(fn));
    TS_ASSERT(h.getHullFunctions(false).findEntry(fn) == 0);

    h.clearHullFunctions();
    TS_ASSERT(h.getHullFunctions(true).findEntry(fn) == 0);
    TS_ASSERT(ch.getHullFunctions(true).findEntry(fn) == 0);
}

/** Test getTurnFuelUsage(). */
void
TestGameSpecHull::testFuelUsage()
{
    // Values verified using c2hosttest/ship/02_fuelperturn
    game::config::HostConfiguration config;
    config[game::config::HostConfiguration::FuelUsagePerTurnFor100KT].set(5);

    game::spec::Hull t(1);

    // Outrider (75 kt) will burn 4 kt
    t.setMass(75);
    TS_ASSERT_EQUALS(t.getTurnFuelUsage(1, false, config), 4);

    // Banshee (120 kt) will burn 6 kt
    t.setMass(120);
    TS_ASSERT_EQUALS(t.getTurnFuelUsage(1, false, config), 6);

    // Loki (101 kt) will burn 6 kt
    t.setMass(101);
    TS_ASSERT_EQUALS(t.getTurnFuelUsage(1, false, config), 6);

    // NFC (10 kt) will burn 1 kt
    t.setMass(10);
    TS_ASSERT_EQUALS(t.getTurnFuelUsage(1, false, config), 1);

    // Dark Wing (491 kt) will burn 25 kt
    t.setMass(491);
    TS_ASSERT_EQUALS(t.getTurnFuelUsage(1, false, config), 25);
}

/** Test getCloakFuelUsage. */
void
TestGameSpecHull::testCloakFuelUsage()
{
    // Values verified using c2hosttest/ship/02_fuelperturn
    game::config::HostConfiguration config;
    config[game::config::HostConfiguration::CloakFuelBurn].set(5);

    game::spec::Hull t(1);

    // BR4 (55 kt) will burn 5 kt
    t.setMass(55);
    TS_ASSERT_EQUALS(t.getCloakFuelUsage(1, config), 5);

    // LCC (160 kt) will burn 8 kt
    t.setMass(160);
    TS_ASSERT_EQUALS(t.getCloakFuelUsage(1, config), 8);

    // Death Specula (113 kt) will burn 5 kt
    t.setMass(113);
    TS_ASSERT_EQUALS(t.getCloakFuelUsage(1, config), 5);
}

/** Test getMineHitDamage. */
void
TestGameSpecHull::testMineHitDamage()
{
    // Values verified using c2hosttest/mine/02_damage
    game::config::HostConfiguration config;
    game::HostVersion h(game::HostVersion::Host, MKVERSION(3,22,40));
    game::HostVersion p(game::HostVersion::PHost, MKVERSION(4,0,0));

    game::spec::Hull t(3);

    // T-Rex (#23), 421 kt -> 24% damage in THost, 23% damage in PHost
    t.setMass(421);
    TS_ASSERT_EQUALS(t.getMineHitDamage(1, false, h, config), 24);
    TS_ASSERT_EQUALS(t.getMineHitDamage(1, false, p, config), 23);

    // Banshee (#6), 120 kt -> 83% damage in either host
    t.setMass(120);
    TS_ASSERT_EQUALS(t.getMineHitDamage(1, false, h, config), 83);
    TS_ASSERT_EQUALS(t.getMineHitDamage(1, false, p, config), 83);

    // Bohemian on Webs (#3), 32 kt -> 30% damage in THost, 31% damage in PHost
    t.setMass(32);
    TS_ASSERT_EQUALS(t.getMineHitDamage(1, true, h, config), 30);
    TS_ASSERT_EQUALS(t.getMineHitDamage(1, true, p, config), 31);
}

