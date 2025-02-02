/**
  *  \file test/game/vcr/objecttest.cpp
  *  \brief Test for game::vcr::Object
  */

#include "game/vcr/object.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/componentvector.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

/** Test "get/set" methods. */
AFL_TEST("game.vcr.Object:basics", a)
{
    game::vcr::Object t;
    t.setMass(99);
    a.checkEqual("01. getMass", t.getMass(), 99);

    t.setShield(42);
    a.checkEqual("11. getShield", t.getShield(), 42);

    t.setDamage(3);
    a.checkEqual("21. getDamage", t.getDamage(), 3);

    t.setCrew(2530);
    a.checkEqual("31. getCrew", t.getCrew(), 2530);

    t.setId(499);
    a.checkEqual("41. getId", t.getId(), 499);

    t.setOwner(12);
    a.checkEqual("51. getOwner", t.getOwner(), 12);

    t.setRace(2);
    a.checkEqual("61. getRace", t.getRace(), 2);

    t.setPicture(200);
    a.checkEqual("71. getPicture", t.getPicture(), 200);

    t.setHull(105);
    a.checkEqual("81. getHull", t.getHull(), 105);

    t.setBeamType(8);
    a.checkEqual("91. getBeamType", t.getBeamType(), 8);

    t.setNumBeams(15);
    a.checkEqual("101. getNumBeams", t.getNumBeams(), 15);

    t.setTorpedoType(3);
    a.checkEqual("111. getTorpedoType", t.getTorpedoType(), 3);

    t.setNumTorpedoes(600);
    a.checkEqual("121. getNumTorpedoes", t.getNumTorpedoes(), 600);

    t.setNumLaunchers(19);
    a.checkEqual("131. getNumLaunchers", t.getNumLaunchers(), 19);

    t.setNumBays(14);
    a.checkEqual("141. getNumBays", t.getNumBays(), 14);

    t.setNumFighters(400);
    a.checkEqual("151. getNumFighters", t.getNumFighters(), 400);

    t.setExperienceLevel(4);
    a.checkEqual("161. getExperienceLevel", t.getExperienceLevel(), 4);

    // The following are initialized to defaults:
    a.checkEqual("171. getBeamKillRate", t.getBeamKillRate(), 1);
    t.setBeamKillRate(3);
    a.checkEqual("172. getBeamKillRate", t.getBeamKillRate(), 3);

    a.checkEqual("181. getBeamChargeRate", t.getBeamChargeRate(), 1);
    t.setBeamChargeRate(2);
    a.checkEqual("182. getBeamChargeRate", t.getBeamChargeRate(), 2);

    a.checkEqual("191. getTorpMissRate", t.getTorpMissRate(), 35);
    t.setTorpMissRate(20);
    a.checkEqual("192. getTorpMissRate", t.getTorpMissRate(), 20);

    a.checkEqual("201. getTorpChargeRate", t.getTorpChargeRate(), 1);
    t.setTorpChargeRate(3);
    a.checkEqual("202. getTorpChargeRate", t.getTorpChargeRate(), 3);

    a.checkEqual("211. getCrewDefenseRate", t.getCrewDefenseRate(), 0);
    t.setCrewDefenseRate(10);
    a.checkEqual("212. getCrewDefenseRate", t.getCrewDefenseRate(), 10);

    a.checkEqual("221. getRole", t.getRole(), t.NoRole);
    t.setRole(game::vcr::Object::AggressorRole);
    a.checkEqual("222. getRole", t.getRole(), game::vcr::Object::AggressorRole);

    t.setIsPlanet(true);
    a.check("231. isPlanet", t.isPlanet());
    t.setIsPlanet(false);
    a.check("232. isPlanet", !t.isPlanet());

    t.setName("NSEA Protector");
    a.checkEqual("241. getName", t.getName(), "NSEA Protector");
}

/* Test getNonEmptyName, ship */
AFL_TEST("game.vcr.Object:name:ship", a)
{
    afl::string::NullTranslator tx;
    game::vcr::Object t;
    t.setIsPlanet(false);
    t.setName("");
    t.setId(42);
    a.checkEqual("getNonEmptyName", t.getNonEmptyName(tx), "Ship 42");
}

/* Test getNonEmptyName, planet */
AFL_TEST("game.vcr.Object:name:planet", a)
{
    afl::string::NullTranslator tx;
    game::vcr::Object t;
    t.setIsPlanet(true);
    t.setName("");
    t.setId(363);
    a.checkEqual("getNonEmptyName", t.getNonEmptyName(tx), "Planet 363");
}

/** Test "add" methods. */
AFL_TEST("game.vcr.Object:add", a)
{
    game::vcr::Object t;

    t.setNumFighters(4);
    a.checkEqual("01. getNumFighters", t.getNumFighters(), 4);
    t.addFighters(12);
    a.checkEqual("02. getNumFighters", t.getNumFighters(), 16);
    t.addFighters(-1);
    a.checkEqual("03. getNumFighters", t.getNumFighters(), 15);

    t.setNumTorpedoes(10);
    a.checkEqual("11. getNumTorpedoes", t.getNumTorpedoes(), 10);
    t.addTorpedoes(430);
    a.checkEqual("12. getNumTorpedoes", t.getNumTorpedoes(), 440);
    t.addTorpedoes(-99);
    a.checkEqual("13. getNumTorpedoes", t.getNumTorpedoes(), 341);

    t.setNumBays(3);
    a.checkEqual("21. getNumBays", t.getNumBays(), 3);
    t.addBays(4);
    a.checkEqual("22. getNumBays", t.getNumBays(), 7);

    t.setMass(100);
    a.checkEqual("31. getMass", t.getMass(), 100);
    t.addMass(340);
    a.checkEqual("32. getMass", t.getMass(), 440);
}

/** Test guessing the ship type. */
AFL_TEST("game.vcr.Object:hull-type-guessing", a)
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
    a.check("01. hull created", p);
    p->setMass(300);
    p->setMaxBeams(11);
    p->setMaxLaunchers(3);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(44);

    p = vec.create(10);
    a.check("11. hull created", p);
    p->setMass(300);
    p->setMaxBeams(12);
    p->setNumBays(1);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(77);

    a.check("21. canBeHull", !testee.canBeHull(vec, 1));
    a.check("22. canBeHull", !testee.canBeHull(vec, 2));
    a.check("23. canBeHull", testee.canBeHull(vec, 10));
    a.checkEqual("24. getGuessedHull", testee.getGuessedHull(vec), 10);
    a.checkEqual("25. getGuessedShipPicture", testee.getGuessedShipPicture(vec), 77);

    testee.setGuessedHull(vec);
    a.checkEqual("31. getHull", testee.getHull(), 10);
}

/** Test guessing the ship type, ambiguous case. */
AFL_TEST("game.vcr.Object:hull-type-guessing:ambiguous", a)
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
    a.check("01. hull created", p);
    p->setMass(300);
    p->setMaxBeams(14);
    p->setNumBays(3);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(44);

    p = vec.create(10);
    a.check("11. hull created", p);
    p->setMass(300);
    p->setMaxBeams(12);
    p->setNumBays(1);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(77);

    a.check("21. canBeHull", testee.canBeHull(vec, 1));
    a.check("22. canBeHull", testee.canBeHull(vec, 10));
    a.checkEqual("23. getGuessedHull", testee.getGuessedHull(vec), 0);
    a.checkEqual("24. getGuessedShipPicture", testee.getGuessedShipPicture(vec), 3);

    // Manually resolve the ambiguity
    testee.setHull(1);
    a.check("31. canBeHull", testee.canBeHull(vec, 1));
    a.check("32. canBeHull", !testee.canBeHull(vec, 10));
    a.checkEqual("33. getGuessedHull", testee.getGuessedHull(vec), 1);
    a.checkEqual("34. getGuessedShipPicture", testee.getGuessedShipPicture(vec), 44);
}

/** Test guessing the ship type, total mismatch. */
AFL_TEST("game.vcr.Object:hull-type-guessing:mismatch", a)
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
    a.check("01. hull created", p);
    p->setMass(300);
    p->setMaxBeams(10);
    p->setNumBays(3);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(44);

    p = vec.create(10);
    a.check("11. hull created", p);
    p->setMass(300);
    p->setMaxBeams(12);
    p->setMaxLaunchers(2);
    p->setExternalPictureNumber(3);
    p->setInternalPictureNumber(77);

    a.check("21. canBeHull", !testee.canBeHull(vec, 1));
    a.check("22. canBeHull", !testee.canBeHull(vec, 10));
    a.checkEqual("23. getGuessedHull", testee.getGuessedHull(vec), 0);
    a.checkEqual("24. getGuessedShipPicture", testee.getGuessedShipPicture(vec), 3);

    // Manually resolve; this will skip the consistency checks
    testee.setHull(1);
    a.check("31. canBeHull", testee.canBeHull(vec, 1));
    a.check("32. canBeHull", !testee.canBeHull(vec, 10));
    a.checkEqual("33. getGuessedHull", testee.getGuessedHull(vec), 1);
    a.checkEqual("34. getGuessedShipPicture", testee.getGuessedShipPicture(vec), 44);
}

/** Test engine guessing. */
AFL_TEST("game.vcr.Object:getGuessedEngine", a)
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
        a.checkEqual("01", obj.getGuessedEngine(engines, &hull, true, config), 9);
    }

    // Success case including 360k bonus
    {
        game::vcr::Object obj;
        obj.setMass(230 + 360);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        obj.setNumBays(1);
        a.checkEqual("11", obj.getGuessedEngine(engines, &hull, true, config), 9);
    }

    // Success case including scotty bonus
    {
        game::vcr::Object obj;
        obj.setMass(230 + 50);
        obj.setIsPlanet(false);
        obj.setOwner(1);
        a.checkEqual("21", obj.getGuessedEngine(engines, &hull, true, config), 9);
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
        a.checkEqual("31", obj.getGuessedEngine(engines, &hull, true, localConfig), 7);
    }

    // Failure case: planet
    {
        game::vcr::Object obj;
        obj.setMass(230);
        obj.setIsPlanet(true);
        obj.setOwner(3);
        a.checkEqual("41", obj.getGuessedEngine(engines, &hull, true, config), 0);
    }

    // Failure case: no hull
    {
        game::vcr::Object obj;
        obj.setMass(230);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        a.checkEqual("51", obj.getGuessedEngine(engines, 0, true, config), 0);
    }

    // Failure case: ESB disabled
    {
        game::vcr::Object obj;
        obj.setMass(230);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        a.checkEqual("61", obj.getGuessedEngine(engines, &hull, false, config), 0);
    }

    // Failure case: no 360k bonus because no fighters
    {
        game::vcr::Object obj;
        obj.setMass(230 + 360);
        obj.setIsPlanet(false);
        obj.setOwner(3);
        a.checkEqual("71", obj.getGuessedEngine(engines, &hull, true, config), 0);
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
        a.checkEqual("81", obj.getGuessedEngine(localEngines, &hull, true, config), 0);
    }
}

/** Test describe(). */
AFL_TEST("game.vcr.Object:describe", a)
{
    // TeamSettings
    game::TeamSettings teamSettings;
    teamSettings.setPlayerTeam(2, 1);
    teamSettings.setViewpointPlayer(1);

    // Root
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))));

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

        a.checkEqual("01", info.text[0], "N1");
    }

    // Ultra lo-fi case
    {
        game::vcr::Object obj;
        obj.setOwner(1);
        obj.setId(77);
        game::vcr::ObjectInfo info = obj.describe(0, 0, 0, tx);

        a.checkEqual("01", info.text[0], "Ship 77");
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
        game::vcr::ObjectInfo info = obj.describe(0, &*root, &shipList, tx);

        a.checkEqual("11", info.text[0], "N2 (Id #77, a Player 1 OUTRIDER CLASS SCOUT)");
        a.checkEqual("12", info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        a.checkEqual("13", info.color[0], util::SkinColor::Static);
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("21", info.text[0], "N3 (Id #77, our OUTRIDER CLASS SCOUT)");
        a.checkEqual("22", info.color[0], util::SkinColor::Green);
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("31", info.text[0], "N4 (Id #77, a Player 2 OUTRIDER CLASS SCOUT)");
        a.checkEqual("32", info.color[0], util::SkinColor::Yellow);
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("41", info.text[0], "N5 (Id #77, a Player 3 OUTRIDER CLASS SCOUT)");
        a.checkEqual("42", info.color[0], util::SkinColor::Red);
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("51", info.text[0], "N6 (Id #77, a Player 3 starship)");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("61", info.text[0], "N7 (Id #77, our planet)");
        a.checkEqual("62", info.text[1], "50% shield (175 kt), 3% damaged");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("71", info.text[0], "N8 (Id #77, our starship)");
        a.checkEqual("72", info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        a.checkEqual("73", info.text[2], "3 \xC3\x97 Heavy Phaser");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("81", info.text[0], "N8 (Id #77, our starship)");
        a.checkEqual("82", info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        a.checkEqual("83", info.text[2], "3 beam weapons");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("91", info.text[0], "N9 (Id #77, our starship)");
        a.checkEqual("92", info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        a.checkEqual("93", info.text[2], "1 \xC3\x97 Mark 2 Photon launcher with 10 torpedoes");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("101", info.text[0], "N10 (Id #77, our starship)");
        a.checkEqual("102", info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        a.checkEqual("103", info.text[2], "10 \xC3\x97 Mark 2 Photon launchers with 1 torpedo");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("111", info.text[0], "N11 (Id #77, our starship)");
        a.checkEqual("112", info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        a.checkEqual("113", info.text[2], "1 \xC3\x97 torpedo launcher with 10 torpedoes");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("121", info.text[0], "N12 (Id #77, our starship)");
        a.checkEqual("122", info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen");
        a.checkEqual("123", info.text[2], "4 fighter bays with 30 fighters");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("131", info.text[0], "N13 (Id #77, our planet)");
        a.checkEqual("132", info.text[1], "0% shield (200 kt), 0% damaged");
        a.checkEqual("133", info.text[2], "20 Mark 8 Photons and 30 fighters");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("141", info.text[0], "N14 (Id #77, our planet)");
        a.checkEqual("142", info.text[1], "0% shield (200 kt), 0% damaged");
        a.checkEqual("143", info.text[2], "20 torpedoes and 30 fighters");
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("151", info.text[0], "N15 (Id #77, our GORBIE CLASS BATTLECARRIER)");
        a.checkEqual("152", info.text[1], "0% shield (980 kt), 0% damaged, 10 crewmen");
        a.checkEqual("153", info.text[2], "4 \xC3\x97 Heavy Blaster");
        a.checkEqual("154", info.text[3], "(10 fighter bays not used)");
        a.checkEqual("155", info.color[3], util::SkinColor::Faded);
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("161", info.text[0], "N16 (Id #77, our GORBIE CLASS BATTLECARRIER)");
        a.checkEqual("162", info.text[1], "0% shield (980 kt), 0% damaged, 10 crewmen");
        a.checkEqual("163", info.text[2], "4 \xC3\x97 Heavy Blaster");
        a.checkEqual("164", info.text[3], "(10 fighter bays with 66 fighters not used)");
        a.checkEqual("165", info.color[3], util::SkinColor::Faded);
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("171", info.text[0], "N17 (Id #77, our ANNIHILATION CLASS BATTLESHIP)");
        a.checkEqual("172", info.text[1], "0% shield (960 kt), 0% damaged, 10 crewmen");
        a.checkEqual("173", info.text[2], "4 \xC3\x97 Heavy Blaster");
        a.checkEqual("174", info.text[3], "(up to 10 torpedo launchers not used)");
        a.checkEqual("175", info.color[3], util::SkinColor::Faded);
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
        game::vcr::ObjectInfo info = obj.describe(&teamSettings, &*root, &shipList, tx);

        a.checkEqual("181", info.text[0], "N18 (Id #77, our ANNIHILATION CLASS BATTLESHIP)");
        a.checkEqual("182", info.text[1], "0% shield (960 kt), 0% damaged, 10 crewmen");
        a.checkEqual("183", info.text[2], "4 \xC3\x97 Heavy Blaster");
        a.checkEqual("184", info.text[3], "(up to 10 Mark 3 Photons with 33 torps not used)");
        a.checkEqual("185", info.color[3], util::SkinColor::Faded);
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
        game::vcr::ObjectInfo info = obj.describe(0, &*root, &shipList, tx);

        a.checkEqual("191", info.text[0], "N19 (Id #77, a Player 1 OUTRIDER CLASS SCOUT)");
        a.checkEqual("192", info.text[1], "0% shield (75 kt), 0% damaged, 10 crewmen, aggressor");
        a.checkEqual("193", info.color[0], util::SkinColor::Static);
    }
}
