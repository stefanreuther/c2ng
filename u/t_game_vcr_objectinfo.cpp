/**
  *  \file u/t_game_vcr_objectinfo.cpp
  *  \brief Test for game::vcr::ObjectInfo
  */

#include "game/vcr/objectinfo.hpp"

#include "t_game_vcr.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/object.hpp"
#include "util/unicodechars.hpp"

/** Test describePlanet, trivial case.
    A: prepare trivial planet (101 kt, from North Star 4 turn 43 Cyborg). Call describePlanet.
    E: verify correct result */
void
TestGameVcrObjectInfo::testPlanet1()
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

    game::config::HostConfiguration config;

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, config);

    // Verify
    TS_ASSERT_EQUALS(result.isValid, true);
    TS_ASSERT_EQUALS(result.hasBase, false);
    TS_ASSERT_EQUALS(result.mass, 101);
    TS_ASSERT_EQUALS(result.defense.min(), 1);
    TS_ASSERT_EQUALS(result.defense.max(), 1);
    TS_ASSERT_EQUALS(result.baseDefense.min(), 0);
    TS_ASSERT_EQUALS(result.baseDefense.max(), 0);
    TS_ASSERT_EQUALS(result.maxBaseFighters, 0);
    TS_ASSERT_EQUALS(result.maxBaseDefense, 0);
}

/** Test describePlanet, average case.
    A: prepare planet (from Pleiades 13 turn 74 Crystal). Call describePlanet.
    E: verify correct result */
void
TestGameVcrObjectInfo::testPlanet2()
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

    game::config::HostConfiguration config;

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, config);

    // Verify
    TS_ASSERT_EQUALS(result.isValid, true);
    TS_ASSERT_EQUALS(result.hasBase, false);
    TS_ASSERT_EQUALS(result.mass, 183);
    TS_ASSERT_EQUALS(result.defense.min(), 83);
    TS_ASSERT_EQUALS(result.defense.max(), 83);
    TS_ASSERT_EQUALS(result.baseDefense.min(), 0);
    TS_ASSERT_EQUALS(result.baseDefense.max(), 0);
    TS_ASSERT_EQUALS(result.maxBaseFighters, 0);
    TS_ASSERT_EQUALS(result.maxBaseDefense, 0);
}

/** Test describePlanet, complex case.
    A: prepare planet (from qvs0 turn 72 Robot). Call describePlanet.
    E: verify correct result */
void
TestGameVcrObjectInfo::testPlanet3()
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

    game::config::HostConfiguration config;

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, config);

    // Verify
    // PCC1 gets a formula error no this setup.
    TS_ASSERT_EQUALS(result.isValid, true);
    TS_ASSERT_EQUALS(result.hasBase, true);
    TS_ASSERT_EQUALS(result.mass, 281);
    TS_ASSERT_EQUALS(result.defense.min(), 73);
    TS_ASSERT_EQUALS(result.defense.max(), 90);
    TS_ASSERT_EQUALS(result.baseDefense.min(), 91);
    TS_ASSERT_EQUALS(result.baseDefense.max(), 108);
    TS_ASSERT_EQUALS(result.numBaseFighters.min(), 0);
    TS_ASSERT_EQUALS(result.numBaseFighters.max(), 0);
    TS_ASSERT_EQUALS(result.baseBeamTech.min(), 10);
    TS_ASSERT_EQUALS(result.baseBeamTech.max(), 10);
    TS_ASSERT_EQUALS(result.maxBaseFighters, 60);
    TS_ASSERT_EQUALS(result.maxBaseDefense, 200);
}

/** Test describePlanet, complex case.
    A: prepare planet (from Titan 12 turn 68 Crystal). Call describePlanet.
    E: verify correct result */
void
TestGameVcrObjectInfo::testPlanet4()
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

    game::config::HostConfiguration config;

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, config);

    // Verify
    // PCC2 <= 2.0.10 reports unknown base tech but we know it cannot be over 8.
    TS_ASSERT_EQUALS(result.isValid, true);
    TS_ASSERT_EQUALS(result.hasBase, true);
    TS_ASSERT_EQUALS(result.mass, 243);
    TS_ASSERT_EQUALS(result.defense.min(), 111);
    TS_ASSERT_EQUALS(result.defense.max(), 132);
    TS_ASSERT_EQUALS(result.baseDefense.min(), 11);
    TS_ASSERT_EQUALS(result.baseDefense.max(), 32);
    TS_ASSERT_EQUALS(result.numBaseFighters.min(), 33);
    TS_ASSERT_EQUALS(result.numBaseFighters.max(), 33);
    TS_ASSERT_EQUALS(result.baseBeamTech.min(), 1);
    TS_ASSERT_EQUALS(result.baseBeamTech.max(), 8);
    TS_ASSERT_EQUALS(result.maxBaseFighters, 60);
    TS_ASSERT_EQUALS(result.maxBaseDefense, 200);
}

/** Test describePlanet, failure case.
    A: prepare invalid planet: 100 kt mass, but nonzero beams. Call describePlanet.
    E: verify result is reported as invalid */
void
TestGameVcrObjectInfo::testFailPlanet1()
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

    game::config::HostConfiguration config;

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, config);

    // Verify
    TS_ASSERT_EQUALS(result.isValid, false);
}

/** Test describePlanet, failure case.
    A: prepare invalid planet: correct mass but mismatching beam count. Call describePlanet.
    E: verify result is reported as invalid */
void
TestGameVcrObjectInfo::testFailPlanet2()
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

    game::config::HostConfiguration config;

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, config);

    // Verify
    TS_ASSERT_EQUALS(result.isValid, false);
}

/** Test describePlanet, failure case.
    A: prepare ship. Call describePlanet.
    E: verify result is reported as invalid */
void
TestGameVcrObjectInfo::testFailNotPlanet()
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

    game::config::HostConfiguration config;

    // Check
    game::vcr::PlanetInfo result;
    describePlanet(result, o, config);

    // Verify
    TS_ASSERT_EQUALS(result.isValid, false);
}

/** Test describeShip, normal case.
    A: prepare ship. Call describeShip with matching ship list.
    E: verify result */
void
TestGameVcrObjectInfo::testShip()
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

    game::config::HostConfiguration config;
    config[game::config::HostConfiguration::AllowEngineShieldBonus].set(true);
    config[game::config::HostConfiguration::EngineShieldBonusRate].set(20);

    afl::string::NullTranslator tx;
    util::NumberFormatter fmt(true, true);

    // Action
    game::vcr::ShipInfo info;
    describeShip(info, o, shipList, shipList.hulls().get(game::test::ANNIHILATION_HULL_ID), true, config, tx, fmt);

    // Verify
    TS_ASSERT_EQUALS(info.primary.first,           "10 " UTF_TIMES " Heavy Disruptor");
    TS_ASSERT_EQUALS(info.primary.second,          "10 beams");
    TS_ASSERT_EQUALS(info.secondary.first,         "7 " UTF_TIMES " Mark 7 Photon");
    TS_ASSERT_EQUALS(info.secondary.second,        "10 launchers");
    TS_ASSERT_EQUALS(info.ammo.first,              "40 torpedoes");
    TS_ASSERT_EQUALS(info.ammo.second,             "320 kt cargo");
    TS_ASSERT_EQUALS(info.crew.first,              "2,910");
    TS_ASSERT_EQUALS(info.crew.second,             "2,910");
    TS_ASSERT_EQUALS(info.experienceLevel.first,   "");
    TS_ASSERT_EQUALS(info.experienceLevel.second,  "");
    TS_ASSERT_EQUALS(info.techLevel.first,         "");
    TS_ASSERT_EQUALS(info.techLevel.second,        "10");
    TS_ASSERT_EQUALS(info.mass.first,              "1,020 kt");
    TS_ASSERT_EQUALS(info.mass.second,             "960 kt");
    TS_ASSERT_EQUALS(info.shield.first,            "100%");
    TS_ASSERT_EQUALS(info.shield.second,           "");
    TS_ASSERT_EQUALS(info.damage.first,            "0%");
    TS_ASSERT_EQUALS(info.damage.second,           "99%");
    TS_ASSERT_EQUALS(info.fuel.first,              "");
    TS_ASSERT_EQUALS(info.fuel.second,             "1,260 kt");
    TS_ASSERT_EQUALS(info.engine.first,            "Transwarp Drive");
    TS_ASSERT_EQUALS(info.engine.second,           "6 engines");
}

/** Test describeShip, hull mismatch case.
    A: prepare ship. Call describeShip with no hull.
    E: verify result */
void
TestGameVcrObjectInfo::testShip2()
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

    game::config::HostConfiguration config;
    afl::string::NullTranslator tx;
    util::NumberFormatter fmt(true, true);

    // Action
    game::vcr::ShipInfo info;
    describeShip(info, o, shipList, 0, true, config, tx, fmt);

    // Verify
    TS_ASSERT_EQUALS(info.primary.first,           "10 " UTF_TIMES " Heavy Phaser");
    TS_ASSERT_EQUALS(info.primary.second,          "");
    TS_ASSERT_EQUALS(info.secondary.first,         "10 fighter bays");
    TS_ASSERT_EQUALS(info.secondary.second,        "");
    TS_ASSERT_EQUALS(info.ammo.first,              "320 fighters");
    TS_ASSERT_EQUALS(info.ammo.second,             "");
    TS_ASSERT_EQUALS(info.crew.first,              "2,810");
    TS_ASSERT_EQUALS(info.crew.second,             "");
    TS_ASSERT_EQUALS(info.experienceLevel.first,   "Soldier");
    TS_ASSERT_EQUALS(info.experienceLevel.second,  "");
    TS_ASSERT_EQUALS(info.techLevel.first,         "");
    TS_ASSERT_EQUALS(info.techLevel.second,        "");
    TS_ASSERT_EQUALS(info.mass.first,              "860 kt");
    TS_ASSERT_EQUALS(info.mass.second,             "");
    TS_ASSERT_EQUALS(info.shield.first,            "100%");
    TS_ASSERT_EQUALS(info.shield.second,           "");
    TS_ASSERT_EQUALS(info.damage.first,            "0%");
    TS_ASSERT_EQUALS(info.damage.second,           "150%");
    TS_ASSERT_EQUALS(info.fuel.first,              "");
    TS_ASSERT_EQUALS(info.fuel.second,             "");
    TS_ASSERT_EQUALS(info.engine.first,            "unknown");
    TS_ASSERT_EQUALS(info.engine.second,           "");
}

