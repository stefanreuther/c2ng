/**
  *  \file u/t_game_sim_ship.cpp
  *  \brief Test for game::sim::Ship
  */

#include "game/sim/ship.hpp"

#include "t_game_sim.hpp"

/** Test getter/setter. */
void
TestGameSimShip::testIt()
{
    game::sim::Ship t;

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

