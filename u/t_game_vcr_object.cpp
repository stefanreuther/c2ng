/**
  *  \file u/t_game_vcr_object.cpp
  *  \brief Test for game::vcr::Object
  */

#include "game/vcr/object.hpp"

#include "t_game_vcr.hpp"

/** Test "get/set" methods. */
void
TestGameVcrObject::testGetSet()
{
    game::vcr::Object t;
    t.setMass(99);
    TS_ASSERT_EQUALS(t.getMass(), 99);

    t.setShield(42);
    TS_ASSERT_EQUALS(t.getShield(), 42);

    t.setDamage(3);
    TS_ASSERT_EQUALS(t.getDamage(), 3);

    t.setCrew(2530);
    TS_ASSERT_EQUALS(t.getCrew(), 2530);

    t.setId(499);
    TS_ASSERT_EQUALS(t.getId(), 499);

    t.setOwner(12);
    TS_ASSERT_EQUALS(t.getOwner(), 12);

    t.setRace(2);
    TS_ASSERT_EQUALS(t.getRace(), 2);

    t.setPicture(200);
    TS_ASSERT_EQUALS(t.getPicture(), 200);

    t.setHull(105);
    TS_ASSERT_EQUALS(t.getHull(), 105);

    t.setBeamType(8);
    TS_ASSERT_EQUALS(t.getBeamType(), 8);

    t.setNumBeams(15);
    TS_ASSERT_EQUALS(t.getNumBeams(), 15);

    t.setTorpedoType(3);
    TS_ASSERT_EQUALS(t.getTorpedoType(), 3);

    t.setNumTorpedoes(600);
    TS_ASSERT_EQUALS(t.getNumTorpedoes(), 600);

    t.setNumLaunchers(19);
    TS_ASSERT_EQUALS(t.getNumLaunchers(), 19);

    t.setNumBays(14);
    TS_ASSERT_EQUALS(t.getNumBays(), 14);

    t.setNumFighters(400);
    TS_ASSERT_EQUALS(t.getNumFighters(), 400);

    t.setExperienceLevel(4);
    TS_ASSERT_EQUALS(t.getExperienceLevel(), 4);

    // The following are initialized to defaults:
    TS_ASSERT_EQUALS(t.getBeamKillRate(), 1);
    t.setBeamKillRate(3);
    TS_ASSERT_EQUALS(t.getBeamKillRate(), 3);

    TS_ASSERT_EQUALS(t.getBeamChargeRate(), 1);
    t.setBeamChargeRate(2);
    TS_ASSERT_EQUALS(t.getBeamChargeRate(), 2);

    TS_ASSERT_EQUALS(t.getTorpMissRate(), 35);
    t.setTorpMissRate(20);
    TS_ASSERT_EQUALS(t.getTorpMissRate(), 20);

    TS_ASSERT_EQUALS(t.getTorpChargeRate(), 1);
    t.setTorpChargeRate(3);
    TS_ASSERT_EQUALS(t.getTorpChargeRate(), 3);

    TS_ASSERT_EQUALS(t.getCrewDefenseRate(), 0);
    t.setCrewDefenseRate(10);
    TS_ASSERT_EQUALS(t.getCrewDefenseRate(), 10);

    t.setIsPlanet(true);
    TS_ASSERT(t.isPlanet());
    t.setIsPlanet(false);
    TS_ASSERT(!t.isPlanet());

    t.setName("NSEA Protector");
    TS_ASSERT_EQUALS(t.getName(), "NSEA Protector");
}

/** Test "add" methods. */
void
TestGameVcrObject::testAdd()
{
    game::vcr::Object t;

    t.setNumFighters(4);
    TS_ASSERT_EQUALS(t.getNumFighters(), 4);
    t.addFighters(12);
    TS_ASSERT_EQUALS(t.getNumFighters(), 16);
    t.addFighters(-1);
    TS_ASSERT_EQUALS(t.getNumFighters(), 15);

    t.setNumTorpedoes(10);
    TS_ASSERT_EQUALS(t.getNumTorpedoes(), 10);
    t.addTorpedoes(430);
    TS_ASSERT_EQUALS(t.getNumTorpedoes(), 440);
    t.addTorpedoes(-99);
    TS_ASSERT_EQUALS(t.getNumTorpedoes(), 341);

    t.setNumBays(3);
    TS_ASSERT_EQUALS(t.getNumBays(), 3);
    t.addBays(4);
    TS_ASSERT_EQUALS(t.getNumBays(), 7);

    t.setMass(100);
    TS_ASSERT_EQUALS(t.getMass(), 100);
    t.addMass(340);
    TS_ASSERT_EQUALS(t.getMass(), 440);
}
