/**
  *  \file u/t_game_sim_planet.cpp
  *  \brief Test for game::sim::Planet
  */

#include "game/sim/planet.hpp"

#include "t_game_sim.hpp"

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
