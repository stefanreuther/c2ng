/**
  *  \file test/game/vcr/objectinfotest.cpp
  *  \brief Test for game::vcr::ObjectInfo
  */

#include "game/vcr/objectinfo.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/object.hpp"
#include "util/unicodechars.hpp"

using afl::base::Ref;
using game::config::HostConfiguration;

/** Test describePlanet, trivial case.
    A: prepare trivial planet (101 kt, from North Star 4 turn 43 Cyborg). Call describePlanet.
    E: verify correct result */
AFL_TEST("game.vcr.ObjectInfo:describePlanet:normal", a)
{
    // Prepare
    game::vcr::Object o;
    o.setMass(101);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(0);
    o.setId(456);
    o.setOwner(5);
    o.setBeamType(1);
    o.setNumBeams(1);
    o.setTorpedoType(1);
    o.setNumLaunchers(1);
    o.setNumBays(1);
    o.setNumFighters(1);
    o.setExperienceLevel(0);
    o.setIsPlanet(true);

    Ref<HostConfiguration> config = HostConfiguration::create();

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, *config);

    // Verify
    a.checkEqual("01. isValid",         result.isValid, true);
    a.checkEqual("02. hasBase",         result.hasBase, false);
    a.checkEqual("03. mass",            result.mass, 101);
    a.checkEqual("04. defense min",     result.defense.min(), 1);
    a.checkEqual("05. defense max",     result.defense.max(), 1);
    a.checkEqual("06. baseDefense min", result.baseDefense.min(), 0);
    a.checkEqual("07. baseDefense max", result.baseDefense.max(), 0);
    a.checkEqual("08. maxBaseFighters", result.maxBaseFighters, 0);
    a.checkEqual("09. maxBaseDefense",  result.maxBaseDefense, 0);
}

/** Test describePlanet, average case.
    A: prepare planet (from Pleiades 13 turn 74 Crystal). Call describePlanet.
    E: verify correct result */
AFL_TEST("game.vcr.ObjectInfo:describePlanet:average", a)
{
    // Prepare
    game::vcr::Object o;
    o.setMass(183);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(0);
    o.setId(20);
    o.setOwner(7);
    o.setBeamType(6);
    o.setNumBeams(5);
    o.setTorpedoType(6);
    o.setNumLaunchers(5);
    o.setNumBays(9);
    o.setNumFighters(9);
    o.setExperienceLevel(1);
    o.setIsPlanet(true);

    Ref<HostConfiguration> config = HostConfiguration::create();

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, *config);

    // Verify
    a.checkEqual("01. isValid",         result.isValid, true);
    a.checkEqual("02. hasBase",         result.hasBase, false);
    a.checkEqual("03. mass",            result.mass, 183);
    a.checkEqual("04. defense min",     result.defense.min(), 83);
    a.checkEqual("05. defense max",     result.defense.max(), 83);
    a.checkEqual("06. baseDefense min", result.baseDefense.min(), 0);
    a.checkEqual("07. baseDefense max", result.baseDefense.max(), 0);
    a.checkEqual("08. maxBaseFighters", result.maxBaseFighters, 0);
    a.checkEqual("09. maxBaseDefense",  result.maxBaseDefense, 0);
}

/** Test describePlanet, complex case.
    A: prepare planet (from qvs0 turn 72 Robot). Call describePlanet.
    E: verify correct result */
AFL_TEST("game.vcr.ObjectInfo:describePlanet:complex", a)
{
    // Prepare
    game::vcr::Object o;
    o.setMass(281);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(0);
    o.setId(446);
    o.setOwner(8);
    o.setBeamType(10);
    o.setNumBeams(8);
    o.setTorpedoType(0);
    o.setNumLaunchers(0);
    o.setNumBays(14);
    o.setNumFighters(9);
    o.setExperienceLevel(0);
    o.setIsPlanet(true);

    Ref<HostConfiguration> config = HostConfiguration::create();

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, *config);

    // Verify
    // PCC1 gets a formula error no this setup.
    a.checkEqual("01. isValid",             result.isValid, true);
    a.checkEqual("02. hasBase",             result.hasBase, true);
    a.checkEqual("03. mass",                result.mass, 281);
    a.checkEqual("04. defense min",         result.defense.min(), 73);
    a.checkEqual("05. defense max",         result.defense.max(), 90);
    a.checkEqual("06. baseDefense min",     result.baseDefense.min(), 91);
    a.checkEqual("07. baseDefense max",     result.baseDefense.max(), 108);
    a.checkEqual("08. numBaseFighters min", result.numBaseFighters.min(), 0);
    a.checkEqual("09. numBaseFighters max", result.numBaseFighters.max(), 0);
    a.checkEqual("10. baseBeamTech min",    result.baseBeamTech.min(), 10);
    a.checkEqual("11. baseBeamTech max",    result.baseBeamTech.max(), 10);
    a.checkEqual("12. maxBaseFighters",     result.maxBaseFighters, 60);
    a.checkEqual("13. maxBaseDefense",      result.maxBaseDefense, 200);
}

/** Test describePlanet, complex case.
    A: prepare planet (from Titan 12 turn 68 Crystal). Call describePlanet.
    E: verify correct result */
AFL_TEST("game.vcr.ObjectInfo:describePlanet:complex2", a)
{
    // Prepare
    game::vcr::Object o;
    o.setMass(243);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(0);
    o.setId(387);
    o.setOwner(7);
    o.setBeamType(8);
    o.setNumBeams(7);
    o.setTorpedoType(0);
    o.setNumLaunchers(0);
    o.setNumBays(16);
    o.setNumFighters(44);
    o.setExperienceLevel(0);
    o.setIsPlanet(true);

    Ref<HostConfiguration> config = HostConfiguration::create();

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, *config);

    // Verify
    // PCC2 <= 2.0.10 reports unknown base tech but we know it cannot be over 8.
    a.checkEqual("01. isValid",             result.isValid, true);
    a.checkEqual("02. hasBase",             result.hasBase, true);
    a.checkEqual("03. mass",                result.mass, 243);
    a.checkEqual("04. defense min",         result.defense.min(), 111);
    a.checkEqual("05. defense max",         result.defense.max(), 132);
    a.checkEqual("06. baseDefense min",     result.baseDefense.min(), 11);
    a.checkEqual("07. baseDefense max",     result.baseDefense.max(), 32);
    a.checkEqual("08. numBaseFighters min", result.numBaseFighters.min(), 33);
    a.checkEqual("09. numBaseFighters max", result.numBaseFighters.max(), 33);
    a.checkEqual("10. baseBeamTech min",    result.baseBeamTech.min(), 1);
    a.checkEqual("11. baseBeamTech max",    result.baseBeamTech.max(), 8);
    a.checkEqual("12. maxBaseFighters",     result.maxBaseFighters, 60);
    a.checkEqual("13. maxBaseDefense",      result.maxBaseDefense, 200);
}

/** Test describePlanet, failure case.
    A: prepare invalid planet: 100 kt mass, but nonzero beams. Call describePlanet.
    E: verify result is reported as invalid */
AFL_TEST("game.vcr.ObjectInfo:describePlanet:error", a)
{
    // Prepare
    game::vcr::Object o;
    o.setMass(100);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(0);
    o.setId(1);
    o.setOwner(2);
    o.setBeamType(1);     // Impossible: cannot have one beam at 100 kt
    o.setNumBeams(1);
    o.setTorpedoType(0);
    o.setNumLaunchers(0);
    o.setNumBays(0);
    o.setNumFighters(0);
    o.setExperienceLevel(0);
    o.setIsPlanet(true);

    Ref<HostConfiguration> config = HostConfiguration::create();

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, *config);

    // Verify
    a.checkEqual("01. isValid", result.isValid, false);
}

/** Test describePlanet, failure case.
    A: prepare invalid planet: correct mass but mismatching beam count. Call describePlanet.
    E: verify result is reported as invalid */
AFL_TEST("game.vcr.ObjectInfo:describePlanet:error2", a)
{
    // Prepare
    game::vcr::Object o;
    o.setMass(125);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(0);
    o.setId(1);
    o.setOwner(2);
    o.setBeamType(10);
    o.setNumBeams(7);     // Impossible: cannot have 7 beams at 125 kt
    o.setTorpedoType(0);
    o.setNumLaunchers(0);
    o.setNumBays(0);
    o.setNumFighters(0);
    o.setExperienceLevel(0);
    o.setIsPlanet(true);

    Ref<HostConfiguration> config = HostConfiguration::create();

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, *config);

    // Verify
    a.checkEqual("01. isValid", result.isValid, false);
}

/** Test describePlanet, failure case.
    A: prepare ship. Call describePlanet.
    E: verify result is reported as invalid */
AFL_TEST("game.vcr.ObjectInfo:describePlanet:not-planet", a)
{
    // Prepare
    game::vcr::Object o;
    o.setMass(120);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(136);
    o.setId(341);
    o.setOwner(7);
    o.setBeamType(9);
    o.setNumBeams(4);
    o.setTorpedoType(9);
    o.setNumLaunchers(2);
    o.setNumBays(0);
    o.setNumFighters(0);
    o.setNumTorpedoes(27);
    o.setExperienceLevel(0);
    o.setIsPlanet(false);

    Ref<HostConfiguration> config = HostConfiguration::create();

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, *config);

    // Verify
    a.checkEqual("01. isValid", result.isValid, false);
}

/** Test describeShip, normal case.
    A: prepare ship. Call describeShip with matching ship list.
    E: verify result */
AFL_TEST("game.vcr.ObjectInfo:describeShip:normal", a)
{
    // Prepare
    game::vcr::Object o;
    o.setMass(1020);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(2910);
    o.setId(444);
    o.setOwner(6);
    o.setBeamType(9);
    o.setNumBeams(10);
    o.setTorpedoType(8);
    o.setNumLaunchers(7);
    o.setNumBays(0);
    o.setNumFighters(0);
    o.setNumTorpedoes(40);
    o.setExperienceLevel(0);
    o.setIsPlanet(false);
    o.setPicture(84);

    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::test::addTranswarp(shipList);
    game::test::addAnnihilation(shipList);

    Ref<HostConfiguration> config = HostConfiguration::create();
    (*config)[HostConfiguration::AllowEngineShieldBonus].set(true);
    (*config)[HostConfiguration::EngineShieldBonusRate].set(20);

    afl::string::NullTranslator tx;
    util::NumberFormatter fmt(true, true);

    // Action
    game::vcr::ShipInfo info;
    describeShip(info, o, shipList, shipList.hulls().get(game::test::ANNIHILATION_HULL_ID), true, *config, tx, fmt);

    // Verify
    a.checkEqual("01. primary",         info.primary.first,           "10 " UTF_TIMES " Heavy Disruptor");
    a.checkEqual("02. primary",         info.primary.second,          "10 beams");
    a.checkEqual("03. secondary",       info.secondary.first,         "7 " UTF_TIMES " Mark 7 Photon");
    a.checkEqual("04. secondary",       info.secondary.second,        "10 launchers");
    a.checkEqual("05. ammo",            info.ammo.first,              "40 torpedoes");
    a.checkEqual("06. ammo",            info.ammo.second,             "320 kt cargo");
    a.checkEqual("07. crew",            info.crew.first,              "2,910");
    a.checkEqual("08. crew",            info.crew.second,             "2,910");
    a.checkEqual("09. experienceLevel", info.experienceLevel.first,   "");
    a.checkEqual("10. experienceLevel", info.experienceLevel.second,  "");
    a.checkEqual("11. techLevel",       info.techLevel.first,         "");
    a.checkEqual("12. techLevel",       info.techLevel.second,        "10");
    a.checkEqual("13. mass",            info.mass.first,              "1,020 kt");
    a.checkEqual("14. mass",            info.mass.second,             "960 kt");
    a.checkEqual("15. shield",          info.shield.first,            "100%");
    a.checkEqual("16. shield",          info.shield.second,           "");
    a.checkEqual("17. damage",          info.damage.first,            "0%");
    a.checkEqual("18. damage",          info.damage.second,           "99%");
    a.checkEqual("19. fuel",            info.fuel.first,              "");
    a.checkEqual("20. fuel",            info.fuel.second,             "1,260 kt");
    a.checkEqual("21. engine",          info.engine.first,            "Transwarp Drive");
    a.checkEqual("22. engine",          info.engine.second,           "6 engines");
}

/** Test describeShip, hull mismatch case.
    A: prepare ship. Call describeShip with no hull.
    E: verify result */
AFL_TEST("game.vcr.ObjectInfo:describeShip:hull-mismatch", a)
{
    // Prepare
    game::vcr::Object o;
    o.setMass(860);
    o.setShield(100);
    o.setDamage(0);
    o.setCrew(2810);
    o.setId(1);
    o.setOwner(2);
    o.setBeamType(10);
    o.setNumBeams(10);
    o.setTorpedoType(0);
    o.setNumLaunchers(0);
    o.setNumBays(10);
    o.setNumFighters(320);
    o.setNumTorpedoes(0);
    o.setExperienceLevel(1);
    o.setIsPlanet(false);
    o.setPicture(84);

    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);
    game::test::addTranswarp(shipList);
    game::test::addAnnihilation(shipList);

    Ref<HostConfiguration> config = HostConfiguration::create();
    afl::string::NullTranslator tx;
    util::NumberFormatter fmt(true, true);

    // Action
    game::vcr::ShipInfo info;
    describeShip(info, o, shipList, 0, true, *config, tx, fmt);

    // Verify
    a.checkEqual("01. primary",         info.primary.first,           "10 " UTF_TIMES " Heavy Phaser");
    a.checkEqual("02. primary",         info.primary.second,          "");
    a.checkEqual("03. secondary",       info.secondary.first,         "10 fighter bays");
    a.checkEqual("04. secondary",       info.secondary.second,        "");
    a.checkEqual("05. ammo",            info.ammo.first,              "320 fighters");
    a.checkEqual("06. ammo",            info.ammo.second,             "");
    a.checkEqual("07. crew",            info.crew.first,              "2,810");
    a.checkEqual("08. crew",            info.crew.second,             "");
    a.checkEqual("09. experienceLevel", info.experienceLevel.first,   "Soldier");
    a.checkEqual("10. experienceLevel", info.experienceLevel.second,  "");
    a.checkEqual("11. techLevel",       info.techLevel.first,         "");
    a.checkEqual("12. techLevel",       info.techLevel.second,        "");
    a.checkEqual("13. mass",            info.mass.first,              "860 kt");
    a.checkEqual("14. mass",            info.mass.second,             "");
    a.checkEqual("15. shield",          info.shield.first,            "100%");
    a.checkEqual("16. shield",          info.shield.second,           "");
    a.checkEqual("17. damage",          info.damage.first,            "0%");
    a.checkEqual("18. damage",          info.damage.second,           "150%");
    a.checkEqual("19. fuel",            info.fuel.first,              "");
    a.checkEqual("20. fuel",            info.fuel.second,             "");
    a.checkEqual("21. engine",          info.engine.first,            "unknown");
    a.checkEqual("22. engine",          info.engine.second,           "");
}
