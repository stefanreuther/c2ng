/**
  *  \file u/t_game_sim_planet.cpp
  *  \brief Test for game::sim::Planet
  */

#include "game/sim/planet.hpp"

#include "t_game_sim.hpp"
#include "game/sim/configuration.hpp"

/** Test getter/setter. */
void
TestGameSimPlanet::testIt()
{
    game::sim::Planet t;

    // Initial state
    TS_ASSERT_EQUALS(t.getDefense(), 10);
    TS_ASSERT_EQUALS(t.getBaseDefense(), 10);
    TS_ASSERT_EQUALS(t.getBaseBeamTech(), 0);
    TS_ASSERT_EQUALS(t.getBaseTorpedoTech(), 1);
    TS_ASSERT_EQUALS(t.getNumBaseFighters(), 0);
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(-1), 0); // out of range
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(0), 0); // out of range
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(1), 0); // in range
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(10), 0); // in range
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(11), 0); // out of range
    TS_ASSERT(!t.hasBase());

    // Get/Set
    t.markClean();
    t.setDefense(61);
    TS_ASSERT_EQUALS(t.getDefense(), 61);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setBaseDefense(50);
    TS_ASSERT_EQUALS(t.getBaseDefense(), 50);
    TS_ASSERT(t.isDirty());

    t.markClean();                             // repeated -> no change signal
    t.setBaseDefense(50);
    TS_ASSERT_EQUALS(t.getBaseDefense(), 50);
    TS_ASSERT(!t.isDirty());

    t.markClean();
    t.setBaseBeamTech(9);
    TS_ASSERT_EQUALS(t.getBaseBeamTech(), 9);
    TS_ASSERT(t.hasBase());
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setBaseTorpedoTech(4);
    TS_ASSERT_EQUALS(t.getBaseTorpedoTech(), 4);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setNumBaseFighters(40);
    TS_ASSERT_EQUALS(t.getNumBaseFighters(), 40);
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(-1, 10);
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(-1), 0); // out of range
    TS_ASSERT(!t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(0, 10);
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(0), 0); // out of range
    TS_ASSERT(!t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(1, 10);
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(1), 10); // in range
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(10, 3);
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(10), 3); // in range
    TS_ASSERT(t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(11, 9);
    TS_ASSERT_EQUALS(t.getNumBaseTorpedoes(11), 0); // out of range
    TS_ASSERT(!t.isDirty());

    TestGameSimObject::verifyObject(t);
}

/** Test hasAbility(). */
void
TestGameSimPlanet::testAbility()
{
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    game::sim::Planet t;
    game::sim::Configuration opts;

    // Lizards don't...
    t.setOwner(2);
    TS_ASSERT(!t.hasAbility(game::sim::TripleBeamKillAbility, opts, shipList, config));

    // ...but Pirates do have this ability.
    t.setOwner(5);
    TS_ASSERT(t.hasAbility(game::sim::TripleBeamKillAbility, opts, shipList, config));
}

/** Test getNumBaseTorpedoesAsType(). */
void
TestGameSimPlanet::testCost()
{
    // Make a ship list
    game::spec::ShipList shipList;
    for (int i = 1; i <= game::sim::Planet::NUM_TORPEDO_TYPES; ++i) {
        game::spec::TorpedoLauncher* tl = shipList.launchers().create(i);
        tl->cost().set(game::spec::Cost::Money, i);
    }

    // Tester
    game::sim::Planet testee;
    testee.setBaseBeamTech(10);
    testee.setNumBaseTorpedoes(1, 100);
    testee.setNumBaseTorpedoes(4, 4);
    testee.setNumBaseTorpedoes(10, 1);
    // total cost: 126

    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(1, shipList), 126);
    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(2, shipList),  63);
    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(3, shipList),  42);
    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(10, shipList), 12);
}

/** Test getNumBaseTorpedoesAsType, zero cost. */
void
TestGameSimPlanet::testCostZero()
{
    // Make a ship list
    game::spec::ShipList shipList;
    for (int i = 1; i <= game::sim::Planet::NUM_TORPEDO_TYPES; ++i) {
        game::spec::TorpedoLauncher* tl = shipList.launchers().create(i);
        tl->cost().set(game::spec::Cost::Money, i);
    }
    shipList.launchers().get(3)->cost().set(game::spec::Cost::Money, 0);

    // Tester
    game::sim::Planet testee;
    testee.setBaseBeamTech(10);
    testee.setNumBaseTorpedoes(1, 100);
    testee.setNumBaseTorpedoes(3, 10);
    // total cost: 100

    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(1, shipList), 100);
    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(2, shipList),  50);
    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(3, shipList), 100);
}

/** Test getNumBaseTorpedoesAsType, partial ship list. */
void
TestGameSimPlanet::testCostPartial()
{
    // Make a ship list with just 5 torpedo types
    game::spec::ShipList shipList;
    for (int i = 1; i <= 5; ++i) {
        game::spec::TorpedoLauncher* tl = shipList.launchers().create(i);
        tl->cost().set(game::spec::Cost::Money, i);
    }

    // Tester
    game::sim::Planet testee;
    testee.setBaseBeamTech(10);
    testee.setNumBaseTorpedoes(1, 100);
    testee.setNumBaseTorpedoes(4, 4);
    testee.setNumBaseTorpedoes(10, 1);
    // total cost: 116

    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(1, shipList), 116);
    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(2, shipList),  58);
    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(3, shipList),  38);
    TS_ASSERT_EQUALS(testee.getNumBaseTorpedoesAsType(10, shipList), 116);
}

