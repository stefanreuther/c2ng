/**
  *  \file u/t_game_vcr_object.cpp
  *  \brief Test for game::vcr::Object
  */

#include "game/vcr/object.hpp"

#include "t_game_vcr.hpp"
#include "game/spec/componentvector.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/engine.hpp"

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

/** Test guessing the ship type. */
void
TestGameVcrObject::testGuess()
{
    // Create an object
    game::vcr::Object testee;
    testee.setPicture(3);
    testee.setMass(200);
    testee.setNumBeams(12);
    testee.setNumBays(3);
    testee.setIsPlanet(false);

    // Create a ship list and test against that
    game::spec::HullVector_t vec;
    game::spec::Hull* p = vec.create(1);
    TS_ASSERT(p);
    p->setMass(300);
    p->setMaxBeams(11);
    p->setMaxLaunchers(3);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(44);

    p = vec.create(10);
    TS_ASSERT(p);
    p->setMass(300);
    p->setMaxBeams(12);
    p->setNumBays(1);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(77);

    TS_ASSERT(!testee.canBeHull(vec, 1));
    TS_ASSERT(!testee.canBeHull(vec, 2));
    TS_ASSERT(testee.canBeHull(vec, 10));
    TS_ASSERT_EQUALS(testee.getGuessedHull(vec), 10);
    TS_ASSERT_EQUALS(testee.getGuessedShipPicture(vec), 77);

    testee.setGuessedHull(vec);
    TS_ASSERT_EQUALS(testee.getHull(), 10);
}

/** Test guessing the ship type, ambiguous case. */
void
TestGameVcrObject::testGuessAmbig()
{
    // Create an object
    game::vcr::Object testee;
    testee.setPicture(3);
    testee.setMass(200);
    testee.setNumBeams(12);
    testee.setNumBays(3);
    testee.setIsPlanet(false);

    // Create a ship list and test against that
    game::spec::HullVector_t vec;
    game::spec::Hull* p = vec.create(1);
    TS_ASSERT(p);
    p->setMass(300);
    p->setMaxBeams(14);
    p->setNumBays(3);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(44);

    p = vec.create(10);
    TS_ASSERT(p);
    p->setMass(300);
    p->setMaxBeams(12);
    p->setNumBays(1);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(77);

    TS_ASSERT(testee.canBeHull(vec, 1));
    TS_ASSERT(testee.canBeHull(vec, 10));
    TS_ASSERT_EQUALS(testee.getGuessedHull(vec), 0);
    TS_ASSERT_EQUALS(testee.getGuessedShipPicture(vec), 3);

    // Manually resolve the ambiguity
    testee.setHull(1);
    TS_ASSERT(testee.canBeHull(vec, 1));
    TS_ASSERT(!testee.canBeHull(vec, 10));
    TS_ASSERT_EQUALS(testee.getGuessedHull(vec), 1);
    TS_ASSERT_EQUALS(testee.getGuessedShipPicture(vec), 44);
}

/** Test guessing the ship type, total mismatch. */
void
TestGameVcrObject::testGuessMismatch()
{
    // Create an object
    game::vcr::Object testee;
    testee.setPicture(3);
    testee.setMass(200);
    testee.setNumBeams(12);
    testee.setNumBays(3);
    testee.setIsPlanet(false);

    // Create a ship list and test against that
    game::spec::HullVector_t vec;
    game::spec::Hull* p = vec.create(1);
    TS_ASSERT(p);
    p->setMass(300);
    p->setMaxBeams(10);
    p->setNumBays(3);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(44);

    p = vec.create(10);
    TS_ASSERT(p);
    p->setMass(300);
    p->setMaxBeams(12);
    p->setMaxLaunchers(2);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(77);

    TS_ASSERT(!testee.canBeHull(vec, 1));
    TS_ASSERT(!testee.canBeHull(vec, 10));
    TS_ASSERT_EQUALS(testee.getGuessedHull(vec), 0);
    TS_ASSERT_EQUALS(testee.getGuessedShipPicture(vec), 3);

    // Manually resolve; this will skip the consistency checks
    testee.setHull(1);
    TS_ASSERT(testee.canBeHull(vec, 1));
    TS_ASSERT(!testee.canBeHull(vec, 10));
    TS_ASSERT_EQUALS(testee.getGuessedHull(vec), 1);
    TS_ASSERT_EQUALS(testee.getGuessedShipPicture(vec), 44);
}

/** Test engine guessing. */
void
TestGameVcrObject::testGuessEngine()
{
    // Environment
    game::spec::Hull hull(12);
    hull.setMass(200);

    game::spec::EngineVector_t engines;
    game::spec::Engine* en7 = engines.create(7); en7->cost().set(game::spec::Cost::Money, 100);
    game::spec::Engine* en9 = engines.create(9); en9->cost().set(game::spec::Cost::Money, 200);

    game::config::HostConfiguration config;
    config[game::config::HostConfiguration::EngineShieldBonusRate].set(15);

    // Success case
    {
        game::vcr::Object obj;
        obj.setMass(230);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        TS_ASSERT_EQUALS(obj.getGuessedEngine(engines, &hull, true, config), 9);
    }

    // Success case including 360k bonus
    {
        game::vcr::Object obj;
        obj.setMass(230 + 360);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        obj.setNumBays(1);
        TS_ASSERT_EQUALS(obj.getGuessedEngine(engines, &hull, true, config), 9);
    }

    // Success case including scotty bonus
    {
        game::vcr::Object obj;
        obj.setMass(230 + 50);
        obj.setIsPlanet(false);
        obj.setOwner(1);
        TS_ASSERT_EQUALS(obj.getGuessedEngine(engines, &hull, true, config), 9);
    }

    // Success case: disabled ESB but experience enabled
    {
        game::config::HostConfiguration localConfig;
        localConfig[game::config::HostConfiguration::EngineShieldBonusRate].set(0);
        localConfig[game::config::HostConfiguration::EModEngineShieldBonusRate].set("2,4,6,8");
        localConfig[game::config::HostConfiguration::NumExperienceLevels].set(4);

        game::vcr::Object obj;
        obj.setMass(206);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        obj.setExperienceLevel(3);
        TS_ASSERT_EQUALS(obj.getGuessedEngine(engines, &hull, true, localConfig), 7);
    }

    // Failure case: planet
    {
        game::vcr::Object obj;
        obj.setMass(230);
        obj.setIsPlanet(true);
        obj.setOwner(3);
        TS_ASSERT_EQUALS(obj.getGuessedEngine(engines, &hull, true, config), 0);
    }

    // Failure case: no hull
    {
        game::vcr::Object obj;
        obj.setMass(230);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        TS_ASSERT_EQUALS(obj.getGuessedEngine(engines, 0, true, config), 0);
    }

    // Failure case: ESB disabled
    {
        game::vcr::Object obj;
        obj.setMass(230);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        TS_ASSERT_EQUALS(obj.getGuessedEngine(engines, 0, false, config), 0);
    }

    // Failure case: no 360k bonus because no fighters
    {
        game::vcr::Object obj;
        obj.setMass(230 + 360);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        TS_ASSERT_EQUALS(obj.getGuessedEngine(engines, &hull, true, config), 0);
    }

    // Failure case: ambiguous engines
    {
        game::spec::EngineVector_t localEngines;
        game::spec::Engine* en7 = localEngines.create(7); en7->cost().set(game::spec::Cost::Money, 200);
        game::spec::Engine* en9 = localEngines.create(9); en9->cost().set(game::spec::Cost::Money, 200);

        game::vcr::Object obj;
        obj.setMass(230);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        TS_ASSERT_EQUALS(obj.getGuessedEngine(localEngines, &hull, true, config), 0);
    }
}

