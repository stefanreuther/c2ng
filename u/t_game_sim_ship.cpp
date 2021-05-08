/**
  *  \file u/t_game_sim_ship.cpp
  *  \brief Test for game::sim::Ship
  */

#include "game/sim/ship.hpp"

#include "t_game_sim.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/spec/shiplist.hpp"
#include "game/spec/hull.hpp"
#include "game/sim/configuration.hpp"

/** Test getter/setter. */
void
TestGameSimShip::testIt()
{
    game::sim::Ship t;
    game::spec::ShipList sl;

    // Initial state
    TS_ASSERT_EQUALS(t.getCrew(), 10);
    TS_ASSERT_EQUALS(t.getHullType(), 0);
    TS_ASSERT_EQUALS(t.getMass(), 100);
    TS_ASSERT_EQUALS(t.getBeamType(), 0);
    TS_ASSERT_EQUALS(t.getNumBeams(), 0);
    TS_ASSERT_EQUALS(t.getTorpedoType(), 0);
    TS_ASSERT_EQUALS(t.getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(t.getNumBays(), 0);
    TS_ASSERT_EQUALS(t.getAmmo(), 0);
    TS_ASSERT_EQUALS(t.getEngineType(), 1);
    TS_ASSERT_EQUALS(t.getAggressiveness(), game::sim::Ship::agg_Passive);
    TS_ASSERT_EQUALS(t.getInterceptId(), 0);

    TS_ASSERT(t.isCustomShip());
    TS_ASSERT_EQUALS(t.getNumBeamsRange(sl).min(), 0);
    TS_ASSERT_LESS_THAN_EQUALS(20, t.getNumBeamsRange(sl).max());
    TS_ASSERT_EQUALS(t.getNumLaunchersRange(sl).min(), 0);
    TS_ASSERT_LESS_THAN_EQUALS(20, t.getNumLaunchersRange(sl).max());
    TS_ASSERT_EQUALS(t.getNumBaysRange(sl).min(), 0);
    TS_ASSERT_LESS_THAN_EQUALS(20, t.getNumBaysRange(sl).max());

    // Set/get
    t.markClean();
    t.setCrew(42);
    TS_ASSERT_EQUALS(t.getCrew(), 42);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setHullTypeOnly(33);
    TS_ASSERT_EQUALS(t.getHullType(), 33);
    TS_ASSERT(t.isDirty());
    TS_ASSERT(!t.isCustomShip());

    t.markClean();
    t.setMass(130);
    TS_ASSERT_EQUALS(t.getMass(), 130);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setBeamType(3);
    TS_ASSERT_EQUALS(t.getBeamType(), 3);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setNumBeams(9);
    TS_ASSERT_EQUALS(t.getNumBeams(), 9);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setTorpedoType(4);
    TS_ASSERT_EQUALS(t.getTorpedoType(), 4);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setNumLaunchers(8);
    TS_ASSERT_EQUALS(t.getNumLaunchers(), 8);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setNumBays(12);
    TS_ASSERT_EQUALS(t.getNumBays(), 12);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setAmmo(80);
    TS_ASSERT_EQUALS(t.getAmmo(), 80);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setEngineType(9);
    TS_ASSERT_EQUALS(t.getEngineType(), 9);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setAggressiveness(7);
    TS_ASSERT_EQUALS(t.getAggressiveness(), 7);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setInterceptId(815);
    TS_ASSERT_EQUALS(t.getInterceptId(), 815);
    TS_ASSERT(t.isDirty());

    TestGameSimObject::verifyObject(t);
}

/** Test name functions. */
void
TestGameSimShip::testName()
{
    afl::string::NullTranslator tx;
    game::sim::Ship t;
    t.setId(77);

    t.setDefaultName(tx);
    TS_ASSERT(t.hasDefaultName(tx));

    t.setId(42);
    TS_ASSERT(!t.hasDefaultName(tx));

    t.setDefaultName(tx);
    TS_ASSERT(t.hasDefaultName(tx));
}

/** Test hull type / ship list interaction. */
void
TestGameSimShip::testShipList()
{
    // Make a ship list
    game::spec::ShipList list;
    {
        game::spec::Hull* h = list.hulls().create(1);
        h->setMaxFuel(100);
        h->setMaxCrew(50);
        h->setNumEngines(2);
        h->setMaxCargo(80);
        h->setNumBays(5);
        h->setMaxLaunchers(0);
        h->setMaxBeams(15);
        h->setMass(2000);
    }
    {
        game::spec::Hull* h = list.hulls().create(2);
        h->setMaxFuel(200);
        h->setMaxCrew(75);
        h->setNumEngines(3);
        h->setMaxCargo(120);
        h->setNumBays(0);
        h->setMaxLaunchers(10);
        h->setMaxBeams(5);
        h->setMass(3000);
    }
    for (int i = 1; i <= 5; ++i) {
        list.beams().create(i);
    }
    for (int i = 1; i <= 7; ++i) {
        list.launchers().create(i);
    }
    for (int i = 1; i <= 7; ++i) {
        list.engines().create(i);
    }

    // Test
    game::sim::Ship testee;
    testee.setHullType(2, list);
    TS_ASSERT_EQUALS(testee.getHullType(), 2);
    TS_ASSERT_EQUALS(testee.getAmmo(), 120);
    TS_ASSERT_EQUALS(testee.getNumBays(), 0);
    TS_ASSERT_EQUALS(testee.getNumLaunchers(), 10);
    TS_ASSERT_EQUALS(testee.getNumBeams(), 5);
    TS_ASSERT_EQUALS(testee.getTorpedoType(), 7);
    TS_ASSERT_EQUALS(testee.getBeamType(), 5);
    TS_ASSERT_EQUALS(testee.getMass(), 3000);
    TS_ASSERT(testee.isMatchingShipList(list));
    TS_ASSERT_EQUALS(testee.getNumBeamsRange(list).min(), 0);
    TS_ASSERT_EQUALS(testee.getNumBeamsRange(list).max(), 5);
    TS_ASSERT_EQUALS(testee.getNumLaunchersRange(list).min(), 0);
    TS_ASSERT_EQUALS(testee.getNumLaunchersRange(list).max(), 10);
    TS_ASSERT_EQUALS(testee.getNumBaysRange(list).min(), 0);
    TS_ASSERT_EQUALS(testee.getNumBaysRange(list).max(), 0);

    // Vary attributes
    testee.setNumBeams(3);
    TS_ASSERT(testee.isMatchingShipList(list));
    testee.setNumBeams(6);
    TS_ASSERT(!testee.isMatchingShipList(list));
    testee.setNumBeams(5);
    TS_ASSERT(testee.isMatchingShipList(list));

    testee.setNumLaunchers(9);
    TS_ASSERT(testee.isMatchingShipList(list));
    testee.setNumLaunchers(11);
    TS_ASSERT(!testee.isMatchingShipList(list));
    testee.setNumLaunchers(10);
    TS_ASSERT(testee.isMatchingShipList(list));

    testee.setAmmo(1);
    TS_ASSERT(testee.isMatchingShipList(list));
    testee.setAmmo(121);
    TS_ASSERT(!testee.isMatchingShipList(list));
    testee.setAmmo(120);
    TS_ASSERT(testee.isMatchingShipList(list));

    testee.setTorpedoType(0);
    testee.setNumLaunchers(0);
    testee.setNumBays(1);
    TS_ASSERT(!testee.isMatchingShipList(list));
    testee.setNumBays(0);
    TS_ASSERT(testee.isMatchingShipList(list));

    // Change hull type to other existing hull
    testee.setHullType(1, list);
    TS_ASSERT_EQUALS(testee.getHullType(), 1);
    TS_ASSERT_EQUALS(testee.getAmmo(), 80);
    TS_ASSERT_EQUALS(testee.getNumBays(), 5);
    TS_ASSERT_EQUALS(testee.getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(testee.getNumBeams(), 15);
    TS_ASSERT_EQUALS(testee.getTorpedoType(), 0);
    TS_ASSERT_EQUALS(testee.getBeamType(), 5);
    TS_ASSERT_EQUALS(testee.getMass(), 2000);
    TS_ASSERT(testee.isMatchingShipList(list));
    TS_ASSERT_EQUALS(testee.getNumBeamsRange(list).min(), 0);
    TS_ASSERT_EQUALS(testee.getNumBeamsRange(list).max(), 15);
    TS_ASSERT_EQUALS(testee.getNumLaunchersRange(list).min(), 0);
    TS_ASSERT_EQUALS(testee.getNumLaunchersRange(list).max(), 0);
    TS_ASSERT_EQUALS(testee.getNumBaysRange(list).min(), 5);
    TS_ASSERT_EQUALS(testee.getNumBaysRange(list).max(), 5);

    // Vary attributes
    testee.setNumBays(10);
    TS_ASSERT(!testee.isMatchingShipList(list));
    testee.setNumBays(0);
    testee.setNumLaunchers(1);
    testee.setTorpedoType(1);
    TS_ASSERT(!testee.isMatchingShipList(list));
    testee.setNumBays(1);
    testee.setNumLaunchers(0);
    testee.setTorpedoType(0);
    TS_ASSERT(!testee.isMatchingShipList(list));
    testee.setNumBays(5);
    TS_ASSERT(testee.isMatchingShipList(list));

    // Change to nonexistant hull
    testee.setHullType(3, list);
    TS_ASSERT_EQUALS(testee.getHullType(), 3);
    TS_ASSERT(!testee.isMatchingShipList(list));
    TS_ASSERT_EQUALS(testee.getNumBeamsRange(list).min(), 0);
    TS_ASSERT_EQUALS(testee.getNumBeamsRange(list).max(), 0);
    TS_ASSERT_EQUALS(testee.getNumLaunchersRange(list).min(), 0);
    TS_ASSERT_EQUALS(testee.getNumLaunchersRange(list).max(), 0);
    TS_ASSERT_EQUALS(testee.getNumBaysRange(list).min(), 0);
    TS_ASSERT_EQUALS(testee.getNumBaysRange(list).max(), 0);

    // Change to custom ship
    testee.setHullType(0, list);
    TS_ASSERT_EQUALS(testee.getHullType(), 0);
    TS_ASSERT(testee.isMatchingShipList(list));
}

/** Test ship abilities. */
void
TestGameSimShip::testAbilities()
{
    // Make a ship list
    game::spec::ShipList list;
    {
        game::spec::Hull* h = list.hulls().create(1);
        h->changeHullFunction(game::spec::ModifiedHullFunctionList::Function_t(game::spec::HullFunction::Commander),
                              game::PlayerSet_t(9),
                              game::PlayerSet_t(),
                              true);
    }

    // Configuration
    game::config::HostConfiguration config;
    game::sim::Configuration opts;

    game::sim::Configuration nuOpts;
    game::TeamSettings team;
    nuOpts.setMode(game::sim::Configuration::VcrNuHost, team, config);

    // Test
    game::sim::Ship testee;
    testee.setHullType(1, list);

    // Player 1: FullWeaponry
    testee.setOwner(1);
    TS_ASSERT(!testee.hasAnyNonstandardAbility());
    TS_ASSERT( testee.hasAbility(game::sim::FullWeaponryAbility,   opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::PlanetImmunityAbility, opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::TripleBeamKillAbility, opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::CommanderAbility,      opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::ElusiveAbility,        opts, list, config));

    TS_ASSERT_EQUALS(testee.getAbilities(opts, list, config), game::sim::Abilities_t() + game::sim::FullWeaponryAbility);

    // Player 4: PlanetImmunity
    testee.setOwner(4);
    TS_ASSERT(!testee.hasAnyNonstandardAbility());
    TS_ASSERT(!testee.hasAbility(game::sim::FullWeaponryAbility,   opts, list, config));
    TS_ASSERT( testee.hasAbility(game::sim::PlanetImmunityAbility, opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::TripleBeamKillAbility, opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::CommanderAbility,      opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::ElusiveAbility,        opts, list, config));

    TS_ASSERT(!testee.hasAbility(game::sim::DoubleBeamChargeAbility, opts, list, config));
    TS_ASSERT( testee.hasAbility(game::sim::DoubleBeamChargeAbility, nuOpts, list, config));

    TS_ASSERT_EQUALS(testee.getAbilities(opts,   list, config), game::sim::Abilities_t() + game::sim::PlanetImmunityAbility);
    TS_ASSERT_EQUALS(testee.getAbilities(nuOpts, list, config), game::sim::Abilities_t() + game::sim::PlanetImmunityAbility + game::sim::DoubleBeamChargeAbility);

    // Player 5: TripleBeamKill
    testee.setOwner(5);
    TS_ASSERT(!testee.hasAnyNonstandardAbility());
    TS_ASSERT(!testee.hasAbility(game::sim::FullWeaponryAbility,   opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::PlanetImmunityAbility, opts, list, config));
    TS_ASSERT( testee.hasAbility(game::sim::TripleBeamKillAbility, opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::CommanderAbility,      opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::ElusiveAbility,        opts, list, config));

    TS_ASSERT_EQUALS(testee.getAbilities(opts, list, config), game::sim::Abilities_t() + game::sim::TripleBeamKillAbility);

    // Player 9: Commander
    testee.setOwner(9);
    TS_ASSERT(!testee.hasAnyNonstandardAbility());
    TS_ASSERT(!testee.hasAbility(game::sim::FullWeaponryAbility,   opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::PlanetImmunityAbility, opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::TripleBeamKillAbility, opts, list, config));
    TS_ASSERT( testee.hasAbility(game::sim::CommanderAbility,      opts, list, config));
    TS_ASSERT(!testee.hasAbility(game::sim::ElusiveAbility,        opts, list, config));

    TS_ASSERT_EQUALS(testee.getAbilities(opts, list, config), game::sim::Abilities_t() + game::sim::CommanderAbility);
}

void
TestGameSimShip::testAggressive()
{
    using game::sim::Ship;
    TS_ASSERT_EQUALS(Ship::isPrimaryEnemy(0), false);
    TS_ASSERT_EQUALS(Ship::isPrimaryEnemy(Ship::agg_Kill), false);
    TS_ASSERT_EQUALS(Ship::isPrimaryEnemy(Ship::agg_NoFuel), false);
    TS_ASSERT_EQUALS(Ship::isPrimaryEnemy(Ship::agg_Passive), false);

    TS_ASSERT_EQUALS(Ship::isPrimaryEnemy(1), true);
    TS_ASSERT_EQUALS(Ship::isPrimaryEnemy(11), true);
    TS_ASSERT_EQUALS(Ship::isPrimaryEnemy(12), true);
}

