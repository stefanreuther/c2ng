/**
  *  \file u/t_game_vcr_object.cpp
  *  \brief Test for game::vcr::Object
  */

#include "game/vcr/object.hpp"

#include "t_game_vcr.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/spec/componentvector.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

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

    TS_ASSERT_EQUALS(t.getRole(), t.NoRole);
    t.setRole(game::vcr::Object::AggressorRole);
    TS_ASSERT_EQUALS(t.getRole(), game::vcr::Object::AggressorRole);

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

/** Test describe(). */
void
TestGameVcrObject::testDescribe()
{
    // TeamSettings
    game::TeamSettings teamSettings;
    teamSettings.setPlayerTeam(2, 1);
    teamSettings.setViewpointPlayer(1);

    // Root
    game::test::Root root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));

    // ShipList
    game::spec::ShipList shipList;
    game::test::addOutrider(shipList);
    game::test::addGorbie(shipList);
    game::test::addAnnihilation(shipList);
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::test::addTranswarp(shipList);

    // Translator
    afl::string::NullTranslator tx;

    // Lo-fi case
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N1");
        obj.setId(77);
        game::vcr::ObjectInfo info = obj.describe(0, 0, 0, tx);

        TS_ASSERT_EQUALS(info.text[0], "N1");
    }

    // Standard case, no team settings
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N2");
        obj.setId(77);
        obj.setPicture(9);
        obj.setMass(75);
        obj.setCrew(10);
        game::vcr::ObjectInfo info = obj.describe(0, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N2 (Id #77, a Player 1 OUTRIDER CLASS SCOUT)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.color[0], util::SkinColor::Static);
    }

    // Standard case, with team settings, own ship
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N3");
        obj.setId(77);
        obj.setPicture(9);
        obj.setMass(75);
        obj.setCrew(10);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N3 (Id #77, our OUTRIDER CLASS SCOUT)");
        TS_ASSERT_EQUALS(info.color[0], util::SkinColor::Green);
    }

    // Standard case, with team settings, team ship
    {
        game::vcr::Object obj;
        obj.setOwner(2);
        obj.setName("N4");
        obj.setId(77);
        obj.setPicture(9);
        obj.setMass(75);
        obj.setCrew(10);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N4 (Id #77, a Player 2 OUTRIDER CLASS SCOUT)");
        TS_ASSERT_EQUALS(info.color[0], util::SkinColor::Yellow);
    }

    // Standard case, with team settings, enemy ship
    {
        game::vcr::Object obj;
        obj.setOwner(3);
        obj.setName("N5");
        obj.setId(77);
        obj.setPicture(9);
        obj.setMass(75);
        obj.setCrew(10);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N5 (Id #77, a Player 3 OUTRIDER CLASS SCOUT)");
        TS_ASSERT_EQUALS(info.color[0], util::SkinColor::Red);
    }

    // Standard case, unguessable ship
    {
        game::vcr::Object obj;
        obj.setOwner(3);
        obj.setName("N6");
        obj.setId(77);
        obj.setPicture(99);
        obj.setMass(75);
        obj.setCrew(10);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N6 (Id #77, a Player 3 starship)");
    }

    // Standard case, planet
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N7");
        obj.setId(77);
        obj.setPicture(200);
        obj.setMass(175);
        obj.setIsPlanet(true);
        obj.setShield(50);
        obj.setDamage(3);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N7 (Id #77, our planet)");
        TS_ASSERT_EQUALS(info.text[1], "50% shield (175 kt), 3% damaged");
    }

    // Beams
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N8");
        obj.setId(77);
        obj.setPicture(99);
        obj.setMass(75);
        obj.setCrew(10);
        obj.setNumBeams(3);
        obj.setBeamType(10);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N8 (Id #77, our starship)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "3 \xC3\x97 Heavy Phaser");
    }

    // Beams, unknown type
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N8");
        obj.setId(77);
        obj.setPicture(99);
        obj.setMass(75);
        obj.setCrew(10);
        obj.setNumBeams(3);
        obj.setBeamType(0);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N8 (Id #77, our starship)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "3 beam weapons");
    }

    // Torpedoes
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N9");
        obj.setId(77);
        obj.setPicture(99);
        obj.setMass(75);
        obj.setCrew(10);
        obj.setNumLaunchers(1);
        obj.setTorpedoType(3);
        obj.setNumTorpedoes(10);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N9 (Id #77, our starship)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "1 \xC3\x97 Mark 2 Photon launcher with 10 torpedoes");
    }

    // Torpedoes (plural forms)
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N10");
        obj.setId(77);
        obj.setPicture(99);
        obj.setMass(75);
        obj.setCrew(10);
        obj.setNumLaunchers(10);
        obj.setTorpedoType(3);
        obj.setNumTorpedoes(1);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N10 (Id #77, our starship)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "10 \xC3\x97 Mark 2 Photon launchers with 1 torpedo");
    }

    // Torpedoes (unknown type)
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N11");
        obj.setId(77);
        obj.setPicture(99);
        obj.setMass(75);
        obj.setCrew(10);
        obj.setNumLaunchers(1);
        obj.setTorpedoType(0);
        obj.setNumTorpedoes(10);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N11 (Id #77, our starship)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "1 \xC3\x97 torpedo launcher with 10 torpedoes");
    }

    // Fighters
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N12");
        obj.setId(77);
        obj.setPicture(99);
        obj.setMass(75);
        obj.setCrew(10);
        obj.setNumBays(4);
        obj.setNumFighters(30);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N12 (Id #77, our starship)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "4 fighter bays with 30 fighters");
    }

    // Torpedoes and fighters
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N13");
        obj.setId(77);
        obj.setPicture(99);
        obj.setMass(200);
        obj.setIsPlanet(true);
        obj.setNumBays(4);
        obj.setNumFighters(30);
        obj.setTorpedoType(10);
        obj.setNumTorpedoes(20);
        obj.setNumLaunchers(2);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N13 (Id #77, our planet)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (200 kt), 0% damaged");
        TS_ASSERT_EQUALS(info.text[2], "20 Mark 8 Photons and 30 fighters");
    }

    // Torpedoes with unknown type, and fighters
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N14");
        obj.setId(77);
        obj.setPicture(99);
        obj.setMass(200);
        obj.setIsPlanet(true);
        obj.setNumBays(4);
        obj.setNumFighters(30);
        obj.setTorpedoType(0);
        obj.setNumTorpedoes(20);
        obj.setNumLaunchers(2);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N14 (Id #77, our planet)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (200 kt), 0% damaged");
        TS_ASSERT_EQUALS(info.text[2], "20 torpedoes and 30 fighters");
    }

    // Unused bays (THost NTP)
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N15");
        obj.setId(77);
        obj.setPicture(107);      // Picture for GORBIE
        obj.setMass(980);
        obj.setNumBeams(4);
        obj.setBeamType(7);
        obj.setCrew(10);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N15 (Id #77, our GORBIE CLASS BATTLECARRIER)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (980 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "4 \xC3\x97 Heavy Blaster");
        TS_ASSERT_EQUALS(info.text[3], "(10 fighter bays not used)");
        TS_ASSERT_EQUALS(info.color[3], util::SkinColor::Faded);
    }

    // Unused bays (THost NTP), fighters known
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N16");
        obj.setId(77);
        obj.setPicture(107);      // Picture for GORBIE
        obj.setMass(980);
        obj.setNumBeams(4);
        obj.setBeamType(7);
        obj.setCrew(10);
        obj.setNumFighters(66);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N16 (Id #77, our GORBIE CLASS BATTLECARRIER)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (980 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "4 \xC3\x97 Heavy Blaster");
        TS_ASSERT_EQUALS(info.text[3], "(10 fighter bays with 66 fighters not used)");
        TS_ASSERT_EQUALS(info.color[3], util::SkinColor::Faded);
    }

    // Unused torpedo launchers
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N17");
        obj.setId(77);
        obj.setPicture(84);      // Picture for ANNIHILATION
        obj.setMass(960);
        obj.setNumBeams(4);
        obj.setBeamType(7);
        obj.setCrew(10);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N17 (Id #77, our ANNIHILATION CLASS BATTLESHIP)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (960 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "4 \xC3\x97 Heavy Blaster");
        TS_ASSERT_EQUALS(info.text[3], "(up to 10 torpedo launchers not used)");
        TS_ASSERT_EQUALS(info.color[3], util::SkinColor::Faded);
    }

    // Unused torpedo launchers, type/count known
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N18");
        obj.setId(77);
        obj.setPicture(84);      // Picture for ANNIHILATION
        obj.setMass(960);
        obj.setNumBeams(4);
        obj.setBeamType(7);
        obj.setCrew(10);
        obj.setTorpedoType(5);
        obj.setNumTorpedoes(33);
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N18 (Id #77, our ANNIHILATION CLASS BATTLESHIP)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (960 kt), 0% damaged, 10 crewmen");
        TS_ASSERT_EQUALS(info.text[2], "4 \xC3\x97 Heavy Blaster");
        TS_ASSERT_EQUALS(info.text[3], "(up to 10 Mark 3 Photons with 33 torps not used)");
        TS_ASSERT_EQUALS(info.color[3], util::SkinColor::Faded);
    }

    // Standard case, with role
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setName("N19");
        obj.setId(77);
        obj.setPicture(9);
        obj.setMass(75);
        obj.setCrew(10);
        obj.setRole(game::vcr::Object::AggressorRole);
        game::vcr::ObjectInfo info = obj.describe(0, &root, &shipList, tx);

        TS_ASSERT_EQUALS(info.text[0], "N19 (Id #77, a Player 1 OUTRIDER CLASS SCOUT)");
        TS_ASSERT_EQUALS(info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen, aggressor");
        TS_ASSERT_EQUALS(info.color[0], util::SkinColor::Static);
    }
}

