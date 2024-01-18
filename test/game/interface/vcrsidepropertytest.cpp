/**
  *  \file test/game/interface/vcrsidepropertytest.cpp
  *  \brief Test for game::interface::VcrSideProperty
  */

#include "game/interface/vcrsideproperty.hpp"

#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/object.hpp"
#include "game/vcr/test/battle.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    void initShipList(game::spec::ShipList& shipList)
    {
        game::test::initStandardBeams(shipList);
        game::test::initStandardTorpedoes(shipList);
        game::test::addAnnihilation(shipList);
        game::test::addGorbie(shipList);

        for (int i = 1; i <= 10; ++i) {
            shipList.launchers().get(i)->setShortName(afl::string::Format("torp%d", i));
        }
        for (int i = 1; i <= 10; ++i) {
            shipList.beams().get(i)->setShortName(afl::string::Format("beam%d", i));
        }
    }

    void initPlayers(game::PlayerList& players)
    {
        game::Player& p2 = *players.create(2);
        p2.setName(game::Player::ShortName, "The Lizards");
        p2.setName(game::Player::LongName, "The Lizard Empire");
        p2.setName(game::Player::AdjectiveName, "Lizard");

        game::Player& p5 = *players.create(5);
        p5.setName(game::Player::ShortName, "The Pirates");
        p5.setName(game::Player::LongName, "The Pirate Bands");
        p5.setName(game::Player::AdjectiveName, "Pirates");
    }

    game::vcr::Object makeAnnihilation()
    {
        game::vcr::Object o;
        o.setMass(2000);
        o.setShield(98);
        o.setDamage(2);
        o.setCrew(500);
        o.setId(70);
        o.setOwner(2);
        o.setPicture(77);
        o.setHull(game::test::ANNIHILATION_HULL_ID);
        o.setBeamType(5);
        o.setNumBeams(10);
        o.setTorpedoType(3);
        o.setNumLaunchers(7);
        o.setNumTorpedoes(320);
        o.setNumBays(0);
        o.setNumFighters(0);
        o.setExperienceLevel(1);
        o.setBeamKillRate(3);
        o.setBeamChargeRate(1);
        o.setTorpMissRate(40);
        o.setTorpChargeRate(2);
        o.setCrewDefenseRate(10);
        o.setIsPlanet(false);
        o.setName("Anni");
        o.setRole(game::vcr::Object::AggressorRole);
        return o;
    }

    game::vcr::Object makeGorbie()
    {
        game::vcr::Object o;
        o.setMass(1800);
        o.setShield(100);
        o.setDamage(0);
        o.setCrew(700);
        o.setId(90);
        o.setOwner(5);
        o.setPicture(88);
        o.setHull(game::test::GORBIE_HULL_ID);
        o.setBeamType(0);
        o.setNumBeams(0);
        o.setTorpedoType(0);
        o.setNumLaunchers(0);
        o.setNumTorpedoes(0);
        o.setNumBays(8);
        o.setNumFighters(180);
        o.setExperienceLevel(0);
        o.setBeamKillRate(1);
        o.setBeamChargeRate(1);
        o.setTorpMissRate(10);
        o.setTorpChargeRate(2);
        o.setCrewDefenseRate(10);
        o.setIsPlanet(false);
        o.setName("Michal");
        o.setRole(game::vcr::Object::OpponentRole);
        return o;
    }

    game::vcr::Object makeFreighter()
    {
        game::vcr::Object o;
        o.setMass(20);
        o.setShield(0);
        o.setDamage(0);
        o.setCrew(10);
        o.setId(150);
        o.setOwner(5);
        o.setPicture(10);
        o.setHull(0);
        o.setBeamType(0);
        o.setNumBeams(0);
        o.setTorpedoType(0);
        o.setNumLaunchers(0);
        o.setNumTorpedoes(0);
        o.setNumBays(0);
        o.setNumFighters(0);
        o.setExperienceLevel(0);
        o.setBeamKillRate(1);
        o.setBeamChargeRate(1);
        o.setTorpMissRate(10);
        o.setTorpChargeRate(2);
        o.setCrewDefenseRate(10);
        o.setIsPlanet(false);
        o.setName("Cargo");
        o.setRole(game::vcr::Object::NoRole);
        return o;
    }

    game::vcr::Object makePlanet()
    {
        game::vcr::Object o;
        o.setMass(120);
        o.setShield(100);
        o.setDamage(0);
        o.setCrew(0);
        o.setId(363);
        o.setOwner(5);
        o.setPicture(200);
        o.setHull(0);
        o.setBeamType(4);
        o.setNumBeams(4);
        o.setTorpedoType(2);
        o.setNumLaunchers(2);
        o.setNumTorpedoes(12);
        o.setNumBays(5);
        o.setNumFighters(10);
        o.setExperienceLevel(0);
        o.setBeamKillRate(1);
        o.setBeamChargeRate(1);
        o.setTorpMissRate(10);
        o.setTorpChargeRate(2);
        o.setCrewDefenseRate(10);
        o.setIsPlanet(true);
        o.setName("Melmac");
        o.setRole(game::vcr::Object::NoRole);
        return o;
    }
}

AFL_TEST("game.interface.VcrSideProperty", a)
{
    // Environment
    afl::string::NullTranslator tx;
    game::spec::ShipList shipList;
    game::config::HostConfiguration config;
    game::PlayerList players;

    initShipList(shipList);
    initPlayers(players);

    // Battle
    game::vcr::test::Battle b;
    b.addObject(makeAnnihilation(), 3);
    b.addObject(makeGorbie(), -1);
    b.addObject(makePlanet(), 0);
    b.addObject(makeFreighter(), 0);

    // Verify slot 0 (Anni)
    verifyNewInteger(a("ivsAuxAmmo 0"),         getVcrSideProperty(b, 0, game::interface::ivsAuxAmmo,         tx, shipList, config, players), 320);
    verifyNewInteger(a("ivsAuxCount 0"),        getVcrSideProperty(b, 0, game::interface::ivsAuxCount,        tx, shipList, config, players), 7);
    verifyNewInteger(a("ivsAuxId 0"),           getVcrSideProperty(b, 0, game::interface::ivsAuxId,           tx, shipList, config, players), 3);
    verifyNewString (a("ivsAuxName 0"),         getVcrSideProperty(b, 0, game::interface::ivsAuxName,         tx, shipList, config, players), "Mark 2 Photon");
    verifyNewString (a("ivsAuxShort 0"),        getVcrSideProperty(b, 0, game::interface::ivsAuxShort,        tx, shipList, config, players), "torp3");
    verifyNewInteger(a("ivsFighterBays 0"),     getVcrSideProperty(b, 0, game::interface::ivsFighterBays,     tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsFighterCount 0"),    getVcrSideProperty(b, 0, game::interface::ivsFighterCount,    tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsTorpId 0"),          getVcrSideProperty(b, 0, game::interface::ivsTorpId,          tx, shipList, config, players), 3);
    verifyNewInteger(a("ivsTorpCount 0"),       getVcrSideProperty(b, 0, game::interface::ivsTorpCount,       tx, shipList, config, players), 320);
    verifyNewInteger(a("ivsTorpLCount 0"),      getVcrSideProperty(b, 0, game::interface::ivsTorpLCount,      tx, shipList, config, players), 7);
    verifyNewString (a("ivsTorpShort 0"),       getVcrSideProperty(b, 0, game::interface::ivsTorpShort,       tx, shipList, config, players), "torp3");
    verifyNewString (a("ivsTorpName 0"),        getVcrSideProperty(b, 0, game::interface::ivsTorpName,        tx, shipList, config, players), "Mark 2 Photon");
    verifyNewInteger(a("ivsBeamCount 0"),       getVcrSideProperty(b, 0, game::interface::ivsBeamCount,       tx, shipList, config, players), 10);
    verifyNewInteger(a("ivsBeamId 0"),          getVcrSideProperty(b, 0, game::interface::ivsBeamId,          tx, shipList, config, players), 5);
    verifyNewString (a("ivsBeamName 0"),        getVcrSideProperty(b, 0, game::interface::ivsBeamName,        tx, shipList, config, players), "Positron Beam");
    verifyNewString (a("ivsBeamShort 0"),       getVcrSideProperty(b, 0, game::interface::ivsBeamShort,       tx, shipList, config, players), "beam5");
    verifyNewInteger(a("ivsCrew 0"),            getVcrSideProperty(b, 0, game::interface::ivsCrew,            tx, shipList, config, players), 500);
    verifyNewInteger(a("ivsCrewRaw 0"),         getVcrSideProperty(b, 0, game::interface::ivsCrewRaw,         tx, shipList, config, players), 500);
    verifyNewInteger(a("ivsDamage 0"),          getVcrSideProperty(b, 0, game::interface::ivsDamage,          tx, shipList, config, players), 2);
    verifyNewInteger(a("ivsId 0"),              getVcrSideProperty(b, 0, game::interface::ivsId,              tx, shipList, config, players), 70);
    verifyNewInteger(a("ivsMass 0"),            getVcrSideProperty(b, 0, game::interface::ivsMass,            tx, shipList, config, players), 2000);
    verifyNewString (a("ivsName 0"),            getVcrSideProperty(b, 0, game::interface::ivsName,            tx, shipList, config, players), "Anni");
    verifyNewString (a("ivsNameFull 0"),        getVcrSideProperty(b, 0, game::interface::ivsNameFull,        tx, shipList, config, players), "Anni (Ship #70)");
    verifyNewString (a("ivsOwnerAdj 0"),        getVcrSideProperty(b, 0, game::interface::ivsOwnerAdj,        tx, shipList, config, players), "Lizard");
    verifyNewInteger(a("ivsOwnerId 0"),         getVcrSideProperty(b, 0, game::interface::ivsOwnerId,         tx, shipList, config, players), 2);
    verifyNewString (a("ivsOwnerShort 0"),      getVcrSideProperty(b, 0, game::interface::ivsOwnerShort,      tx, shipList, config, players), "The Lizards");
    verifyNewInteger(a("ivsShield 0"),          getVcrSideProperty(b, 0, game::interface::ivsShield,          tx, shipList, config, players), 98);
    verifyNewString (a("ivsStatus 0"),          getVcrSideProperty(b, 0, game::interface::ivsStatus,          tx, shipList, config, players), "Captured");
    verifyNewInteger(a("ivsStatusRaw 0"),       getVcrSideProperty(b, 0, game::interface::ivsStatusRaw,       tx, shipList, config, players), 3);
    verifyNewString (a("ivsType 0"),            getVcrSideProperty(b, 0, game::interface::ivsType,            tx, shipList, config, players), "Torpedo Ship");
    verifyNewString (a("ivsTypeShort 0"),       getVcrSideProperty(b, 0, game::interface::ivsTypeShort,       tx, shipList, config, players), "T");
    verifyNewString (a("ivsHullName 0"),        getVcrSideProperty(b, 0, game::interface::ivsHullName,        tx, shipList, config, players), "ANNIHILATION CLASS BATTLESHIP");
    verifyNewInteger(a("ivsHullId 0"),          getVcrSideProperty(b, 0, game::interface::ivsHullId,          tx, shipList, config, players), game::test::ANNIHILATION_HULL_ID);
    verifyNewInteger(a("ivsImage 0"),           getVcrSideProperty(b, 0, game::interface::ivsImage,           tx, shipList, config, players), 84);
    verifyNewInteger(a("ivsLevel 0"),           getVcrSideProperty(b, 0, game::interface::ivsLevel,           tx, shipList, config, players), 1);
    verifyNewBoolean(a("ivsIsPlanet 0"),        getVcrSideProperty(b, 0, game::interface::ivsIsPlanet,        tx, shipList, config, players), false);
    verifyNewInteger(a("ivsBeamKillRate 0"),    getVcrSideProperty(b, 0, game::interface::ivsBeamKillRate,    tx, shipList, config, players), 3);
    verifyNewInteger(a("ivsBeamChargeRate 0"),  getVcrSideProperty(b, 0, game::interface::ivsBeamChargeRate,  tx, shipList, config, players), 1);
    verifyNewInteger(a("ivsTorpMissRate 0"),    getVcrSideProperty(b, 0, game::interface::ivsTorpMissRate,    tx, shipList, config, players), 40);
    verifyNewInteger(a("ivsTorpChargeRate 0"),  getVcrSideProperty(b, 0, game::interface::ivsTorpChargeRate,  tx, shipList, config, players), 2);
    verifyNewInteger(a("ivsCrewDefenseRate 0"), getVcrSideProperty(b, 0, game::interface::ivsCrewDefenseRate, tx, shipList, config, players), 10);
    verifyNewString (a("ivsRole 0"),            getVcrSideProperty(b, 0, game::interface::ivsRole,            tx, shipList, config, players), "aggressor");

    // Verify slot 1 (Gorbie)
    verifyNewInteger(a("ivsAuxAmmo 1"),         getVcrSideProperty(b, 1, game::interface::ivsAuxAmmo,         tx, shipList, config, players), 180);
    verifyNewInteger(a("ivsAuxCount 1"),        getVcrSideProperty(b, 1, game::interface::ivsAuxCount,        tx, shipList, config, players), 8);
    verifyNewInteger(a("ivsAuxId 1"),           getVcrSideProperty(b, 1, game::interface::ivsAuxId,           tx, shipList, config, players), 11);
    verifyNewString (a("ivsAuxName 1"),         getVcrSideProperty(b, 1, game::interface::ivsAuxName,         tx, shipList, config, players), "Fighters");
    verifyNewString (a("ivsAuxShort 1"),        getVcrSideProperty(b, 1, game::interface::ivsAuxShort,        tx, shipList, config, players), "Ftr");
    verifyNewInteger(a("ivsFighterBays 1"),     getVcrSideProperty(b, 1, game::interface::ivsFighterBays,     tx, shipList, config, players), 8);
    verifyNewInteger(a("ivsFighterCount 1"),    getVcrSideProperty(b, 1, game::interface::ivsFighterCount,    tx, shipList, config, players), 180);
    verifyNewNull   (a("ivsTorpId 1"),          getVcrSideProperty(b, 1, game::interface::ivsTorpId,          tx, shipList, config, players));
    verifyNewInteger(a("ivsTorpCount 1"),       getVcrSideProperty(b, 1, game::interface::ivsTorpCount,       tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsTorpLCount 1"),      getVcrSideProperty(b, 1, game::interface::ivsTorpLCount,      tx, shipList, config, players), 0);
    verifyNewNull   (a("ivsTorpShort 1"),       getVcrSideProperty(b, 1, game::interface::ivsTorpShort,       tx, shipList, config, players));
    verifyNewNull   (a("ivsTorpName 1"),        getVcrSideProperty(b, 1, game::interface::ivsTorpName,        tx, shipList, config, players));
    verifyNewInteger(a("ivsBeamCount 1"),       getVcrSideProperty(b, 1, game::interface::ivsBeamCount,       tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsBeamId 1"),          getVcrSideProperty(b, 1, game::interface::ivsBeamId,          tx, shipList, config, players), 0);
    verifyNewNull   (a("ivsBeamName 1"),        getVcrSideProperty(b, 1, game::interface::ivsBeamName,        tx, shipList, config, players));
    verifyNewNull   (a("ivsBeamShort 1"),       getVcrSideProperty(b, 1, game::interface::ivsBeamShort,       tx, shipList, config, players));
    verifyNewInteger(a("ivsCrew 1"),            getVcrSideProperty(b, 1, game::interface::ivsCrew,            tx, shipList, config, players), 700);
    verifyNewInteger(a("ivsCrewRaw 1"),         getVcrSideProperty(b, 1, game::interface::ivsCrewRaw,         tx, shipList, config, players), 700);
    verifyNewInteger(a("ivsDamage 1"),          getVcrSideProperty(b, 1, game::interface::ivsDamage,          tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsId 1"),              getVcrSideProperty(b, 1, game::interface::ivsId,              tx, shipList, config, players), 90);
    verifyNewInteger(a("ivsMass 1"),            getVcrSideProperty(b, 1, game::interface::ivsMass,            tx, shipList, config, players), 1800);
    verifyNewString (a("ivsName 1"),            getVcrSideProperty(b, 1, game::interface::ivsName,            tx, shipList, config, players), "Michal");
    verifyNewString (a("ivsNameFull 1"),        getVcrSideProperty(b, 1, game::interface::ivsNameFull,        tx, shipList, config, players), "Michal (Ship #90)");
    verifyNewString (a("ivsOwnerAdj 1"),        getVcrSideProperty(b, 1, game::interface::ivsOwnerAdj,        tx, shipList, config, players), "Pirates");
    verifyNewInteger(a("ivsOwnerId 1"),         getVcrSideProperty(b, 1, game::interface::ivsOwnerId,         tx, shipList, config, players), 5);
    verifyNewString (a("ivsOwnerShort 1"),      getVcrSideProperty(b, 1, game::interface::ivsOwnerShort,      tx, shipList, config, players), "The Pirates");
    verifyNewInteger(a("ivsShield 1"),          getVcrSideProperty(b, 1, game::interface::ivsShield,          tx, shipList, config, players), 100);
    verifyNewString (a("ivsStatus 1"),          getVcrSideProperty(b, 1, game::interface::ivsStatus,          tx, shipList, config, players), "Exploded");
    verifyNewInteger(a("ivsStatusRaw 1"),       getVcrSideProperty(b, 1, game::interface::ivsStatusRaw,       tx, shipList, config, players), -1);
    verifyNewString (a("ivsType 1"),            getVcrSideProperty(b, 1, game::interface::ivsType,            tx, shipList, config, players), "Carrier");
    verifyNewString (a("ivsTypeShort 1"),       getVcrSideProperty(b, 1, game::interface::ivsTypeShort,       tx, shipList, config, players), "C");
    verifyNewString (a("ivsHullName 1"),        getVcrSideProperty(b, 1, game::interface::ivsHullName,        tx, shipList, config, players), "GORBIE CLASS BATTLECARRIER");
    verifyNewInteger(a("ivsHullId 1"),          getVcrSideProperty(b, 1, game::interface::ivsHullId,          tx, shipList, config, players), game::test::GORBIE_HULL_ID);
    verifyNewInteger(a("ivsImage 1"),           getVcrSideProperty(b, 1, game::interface::ivsImage,           tx, shipList, config, players), 107);
    verifyNewInteger(a("ivsLevel 1"),           getVcrSideProperty(b, 1, game::interface::ivsLevel,           tx, shipList, config, players), 0);
    verifyNewBoolean(a("ivsIsPlanet 1"),        getVcrSideProperty(b, 1, game::interface::ivsIsPlanet,        tx, shipList, config, players), false);
    verifyNewInteger(a("ivsBeamKillRate 1"),    getVcrSideProperty(b, 1, game::interface::ivsBeamKillRate,    tx, shipList, config, players), 1);
    verifyNewInteger(a("ivsBeamChargeRate 1"),  getVcrSideProperty(b, 1, game::interface::ivsBeamChargeRate,  tx, shipList, config, players), 1);
    verifyNewInteger(a("ivsTorpMissRate 1"),    getVcrSideProperty(b, 1, game::interface::ivsTorpMissRate,    tx, shipList, config, players), 10);
    verifyNewInteger(a("ivsTorpChargeRate 1"),  getVcrSideProperty(b, 1, game::interface::ivsTorpChargeRate,  tx, shipList, config, players), 2);
    verifyNewInteger(a("ivsCrewDefenseRate 1"), getVcrSideProperty(b, 1, game::interface::ivsCrewDefenseRate, tx, shipList, config, players), 10);
    verifyNewString (a("ivsRole 1"),            getVcrSideProperty(b, 1, game::interface::ivsRole,            tx, shipList, config, players), "opponent");

    // Verify slot 2 (planet)
    verifyNewInteger(a("ivsAuxAmmo 2"),         getVcrSideProperty(b, 2, game::interface::ivsAuxAmmo,         tx, shipList, config, players), 10);
    verifyNewInteger(a("ivsAuxCount 2"),        getVcrSideProperty(b, 2, game::interface::ivsAuxCount,        tx, shipList, config, players), 5);
    verifyNewInteger(a("ivsAuxId 2"),           getVcrSideProperty(b, 2, game::interface::ivsAuxId,           tx, shipList, config, players), 11);
    verifyNewString (a("ivsAuxName 2"),         getVcrSideProperty(b, 2, game::interface::ivsAuxName,         tx, shipList, config, players), "Fighters");
    verifyNewString (a("ivsAuxShort 2"),        getVcrSideProperty(b, 2, game::interface::ivsAuxShort,        tx, shipList, config, players), "Ftr");
    verifyNewInteger(a("ivsFighterBays 2"),     getVcrSideProperty(b, 2, game::interface::ivsFighterBays,     tx, shipList, config, players), 5);
    verifyNewInteger(a("ivsFighterCount 2"),    getVcrSideProperty(b, 2, game::interface::ivsFighterCount,    tx, shipList, config, players), 10);
    verifyNewInteger(a("ivsTorpId 2"),          getVcrSideProperty(b, 2, game::interface::ivsTorpId,          tx, shipList, config, players), 2);
    verifyNewInteger(a("ivsTorpCount 2"),       getVcrSideProperty(b, 2, game::interface::ivsTorpCount,       tx, shipList, config, players), 12);
    verifyNewInteger(a("ivsTorpLCount 2"),      getVcrSideProperty(b, 2, game::interface::ivsTorpLCount,      tx, shipList, config, players), 2);
    verifyNewString (a("ivsTorpShort 2"),       getVcrSideProperty(b, 2, game::interface::ivsTorpShort,       tx, shipList, config, players), "torp2");
    verifyNewString (a("ivsTorpName 2"),        getVcrSideProperty(b, 2, game::interface::ivsTorpName,        tx, shipList, config, players), "Proton torp");
    verifyNewInteger(a("ivsBeamCount 2"),       getVcrSideProperty(b, 2, game::interface::ivsBeamCount,       tx, shipList, config, players), 4);
    verifyNewInteger(a("ivsBeamId 2"),          getVcrSideProperty(b, 2, game::interface::ivsBeamId,          tx, shipList, config, players), 4);
    verifyNewString (a("ivsBeamName 2"),        getVcrSideProperty(b, 2, game::interface::ivsBeamName,        tx, shipList, config, players), "Blaster");
    verifyNewString (a("ivsBeamShort 2"),       getVcrSideProperty(b, 2, game::interface::ivsBeamShort,       tx, shipList, config, players), "beam4");
    verifyNewNull   (a("ivsCrew 2"),            getVcrSideProperty(b, 2, game::interface::ivsCrew,            tx, shipList, config, players));
    verifyNewInteger(a("ivsCrewRaw 2"),         getVcrSideProperty(b, 2, game::interface::ivsCrewRaw,         tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsDamage 2"),          getVcrSideProperty(b, 2, game::interface::ivsDamage,          tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsId 2"),              getVcrSideProperty(b, 2, game::interface::ivsId,              tx, shipList, config, players), 363);
    verifyNewInteger(a("ivsMass 2"),            getVcrSideProperty(b, 2, game::interface::ivsMass,            tx, shipList, config, players), 120);
    verifyNewString (a("ivsName 2"),            getVcrSideProperty(b, 2, game::interface::ivsName,            tx, shipList, config, players), "Melmac");
    verifyNewString (a("ivsNameFull 2"),        getVcrSideProperty(b, 2, game::interface::ivsNameFull,        tx, shipList, config, players), "Melmac (Planet #363)");
    verifyNewString (a("ivsOwnerAdj 2"),        getVcrSideProperty(b, 2, game::interface::ivsOwnerAdj,        tx, shipList, config, players), "Pirates");
    verifyNewInteger(a("ivsOwnerId 2"),         getVcrSideProperty(b, 2, game::interface::ivsOwnerId,         tx, shipList, config, players), 5);
    verifyNewString (a("ivsOwnerShort 2"),      getVcrSideProperty(b, 2, game::interface::ivsOwnerShort,      tx, shipList, config, players), "The Pirates");
    verifyNewInteger(a("ivsShield 2"),          getVcrSideProperty(b, 2, game::interface::ivsShield,          tx, shipList, config, players), 100);
    verifyNewString (a("ivsStatus 2"),          getVcrSideProperty(b, 2, game::interface::ivsStatus,          tx, shipList, config, players), "Survived");
    verifyNewInteger(a("ivsStatusRaw 2"),       getVcrSideProperty(b, 2, game::interface::ivsStatusRaw,       tx, shipList, config, players), 0);
    verifyNewString (a("ivsType 2"),            getVcrSideProperty(b, 2, game::interface::ivsType,            tx, shipList, config, players), "Planet");
    verifyNewString (a("ivsTypeShort 2"),       getVcrSideProperty(b, 2, game::interface::ivsTypeShort,       tx, shipList, config, players), "P");
    verifyNewNull   (a("ivsHullName 2"),        getVcrSideProperty(b, 2, game::interface::ivsHullName,        tx, shipList, config, players));
    verifyNewNull   (a("ivsHullId 2"),          getVcrSideProperty(b, 2, game::interface::ivsHullId,          tx, shipList, config, players));
    verifyNewInteger(a("ivsImage 2"),           getVcrSideProperty(b, 2, game::interface::ivsImage,           tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsLevel 2"),           getVcrSideProperty(b, 2, game::interface::ivsLevel,           tx, shipList, config, players), 0);
    verifyNewBoolean(a("ivsIsPlanet 2"),        getVcrSideProperty(b, 2, game::interface::ivsIsPlanet,        tx, shipList, config, players), true);
    verifyNewInteger(a("ivsBeamKillRate 2"),    getVcrSideProperty(b, 2, game::interface::ivsBeamKillRate,    tx, shipList, config, players), 1);
    verifyNewInteger(a("ivsBeamChargeRate 2"),  getVcrSideProperty(b, 2, game::interface::ivsBeamChargeRate,  tx, shipList, config, players), 1);
    verifyNewInteger(a("ivsTorpMissRate 2"),    getVcrSideProperty(b, 2, game::interface::ivsTorpMissRate,    tx, shipList, config, players), 10);
    verifyNewInteger(a("ivsTorpChargeRate 2"),  getVcrSideProperty(b, 2, game::interface::ivsTorpChargeRate,  tx, shipList, config, players), 2);
    verifyNewInteger(a("ivsCrewDefenseRate 2"), getVcrSideProperty(b, 2, game::interface::ivsCrewDefenseRate, tx, shipList, config, players), 10);
    verifyNewNull   (a("ivsRole 2"),            getVcrSideProperty(b, 2, game::interface::ivsRole,            tx, shipList, config, players));

    // Verify slot 3 (freigher)
    verifyNewInteger(a("ivsAuxAmmo 3"),         getVcrSideProperty(b, 3, game::interface::ivsAuxAmmo,         tx, shipList, config, players), 0);
    verifyNewNull   (a("ivsAuxCount 3"),        getVcrSideProperty(b, 3, game::interface::ivsAuxCount,        tx, shipList, config, players));
    verifyNewNull   (a("ivsAuxId 3"),           getVcrSideProperty(b, 3, game::interface::ivsAuxId,           tx, shipList, config, players));
    verifyNewNull   (a("ivsAuxName 3"),         getVcrSideProperty(b, 3, game::interface::ivsAuxName,         tx, shipList, config, players));
    verifyNewNull   (a("ivsAuxShort 3"),        getVcrSideProperty(b, 3, game::interface::ivsAuxShort,        tx, shipList, config, players));
    verifyNewInteger(a("ivsFighterBays 3"),     getVcrSideProperty(b, 3, game::interface::ivsFighterBays,     tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsFighterCount 3"),    getVcrSideProperty(b, 3, game::interface::ivsFighterCount,    tx, shipList, config, players), 0);
    verifyNewNull   (a("ivsTorpId 3"),          getVcrSideProperty(b, 3, game::interface::ivsTorpId,          tx, shipList, config, players));
    verifyNewInteger(a("ivsTorpCount 3"),       getVcrSideProperty(b, 3, game::interface::ivsTorpCount,       tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsTorpLCount 3"),      getVcrSideProperty(b, 3, game::interface::ivsTorpLCount,      tx, shipList, config, players), 0);
    verifyNewNull   (a("ivsTorpShort 3"),       getVcrSideProperty(b, 3, game::interface::ivsTorpShort,       tx, shipList, config, players));
    verifyNewNull   (a("ivsTorpName 3"),        getVcrSideProperty(b, 3, game::interface::ivsTorpName,        tx, shipList, config, players));
    verifyNewInteger(a("ivsBeamCount 3"),       getVcrSideProperty(b, 3, game::interface::ivsBeamCount,       tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsBeamId 3"),          getVcrSideProperty(b, 3, game::interface::ivsBeamId,          tx, shipList, config, players), 0);
    verifyNewNull   (a("ivsBeamName 3"),        getVcrSideProperty(b, 3, game::interface::ivsBeamName,        tx, shipList, config, players));
    verifyNewNull   (a("ivsBeamShort 3"),       getVcrSideProperty(b, 3, game::interface::ivsBeamShort,       tx, shipList, config, players));
    verifyNewInteger(a("ivsCrew 3"),            getVcrSideProperty(b, 3, game::interface::ivsCrew,            tx, shipList, config, players), 10);
    verifyNewInteger(a("ivsCrewRaw 3"),         getVcrSideProperty(b, 3, game::interface::ivsCrewRaw,         tx, shipList, config, players), 10);
    verifyNewInteger(a("ivsDamage 3"),          getVcrSideProperty(b, 3, game::interface::ivsDamage,          tx, shipList, config, players), 0);
    verifyNewInteger(a("ivsId 3"),              getVcrSideProperty(b, 3, game::interface::ivsId,              tx, shipList, config, players), 150);
    verifyNewInteger(a("ivsMass 3"),            getVcrSideProperty(b, 3, game::interface::ivsMass,            tx, shipList, config, players), 20);
    verifyNewString (a("ivsName 3"),            getVcrSideProperty(b, 3, game::interface::ivsName,            tx, shipList, config, players), "Cargo");
    verifyNewString (a("ivsNameFull 3"),        getVcrSideProperty(b, 3, game::interface::ivsNameFull,        tx, shipList, config, players), "Cargo (Ship #150)");
    verifyNewString (a("ivsOwnerAdj 3"),        getVcrSideProperty(b, 3, game::interface::ivsOwnerAdj,        tx, shipList, config, players), "Pirates");
    verifyNewInteger(a("ivsOwnerId 3"),         getVcrSideProperty(b, 3, game::interface::ivsOwnerId,         tx, shipList, config, players), 5);
    verifyNewString (a("ivsOwnerShort 3"),      getVcrSideProperty(b, 3, game::interface::ivsOwnerShort,      tx, shipList, config, players), "The Pirates");
    verifyNewInteger(a("ivsShield 3"),          getVcrSideProperty(b, 3, game::interface::ivsShield,          tx, shipList, config, players), 0);
    verifyNewString (a("ivsStatus 3"),          getVcrSideProperty(b, 3, game::interface::ivsStatus,          tx, shipList, config, players), "Survived");
    verifyNewInteger(a("ivsStatusRaw 3"),       getVcrSideProperty(b, 3, game::interface::ivsStatusRaw,       tx, shipList, config, players), 0);
    verifyNewString (a("ivsType 3"),            getVcrSideProperty(b, 3, game::interface::ivsType,            tx, shipList, config, players), "Freighter");
    verifyNewString (a("ivsTypeShort 3"),       getVcrSideProperty(b, 3, game::interface::ivsTypeShort,       tx, shipList, config, players), "F");
    verifyNewNull   (a("ivsHullName 3"),        getVcrSideProperty(b, 3, game::interface::ivsHullName,        tx, shipList, config, players));
    verifyNewNull   (a("ivsHullId 3"),          getVcrSideProperty(b, 3, game::interface::ivsHullId,          tx, shipList, config, players));
    verifyNewInteger(a("ivsImage 3"),           getVcrSideProperty(b, 3, game::interface::ivsImage,           tx, shipList, config, players), 10);
    verifyNewInteger(a("ivsLevel 3"),           getVcrSideProperty(b, 3, game::interface::ivsLevel,           tx, shipList, config, players), 0);
    verifyNewBoolean(a("ivsIsPlanet 3"),        getVcrSideProperty(b, 3, game::interface::ivsIsPlanet,        tx, shipList, config, players), false);
    verifyNewInteger(a("ivsBeamKillRate 3"),    getVcrSideProperty(b, 3, game::interface::ivsBeamKillRate,    tx, shipList, config, players), 1);
    verifyNewInteger(a("ivsBeamChargeRate 3"),  getVcrSideProperty(b, 3, game::interface::ivsBeamChargeRate,  tx, shipList, config, players), 1);
    verifyNewInteger(a("ivsTorpMissRate 3"),    getVcrSideProperty(b, 3, game::interface::ivsTorpMissRate,    tx, shipList, config, players), 10);
    verifyNewInteger(a("ivsTorpChargeRate 3"),  getVcrSideProperty(b, 3, game::interface::ivsTorpChargeRate,  tx, shipList, config, players), 2);
    verifyNewInteger(a("ivsCrewDefenseRate 3"), getVcrSideProperty(b, 3, game::interface::ivsCrewDefenseRate, tx, shipList, config, players), 10);
    verifyNewNull   (a("ivsRole 3"),            getVcrSideProperty(b, 3, game::interface::ivsRole,            tx, shipList, config, players));

    // Out-of-range
    verifyNewNull   (a("ivsName 4"),            getVcrSideProperty(b, 4, game::interface::ivsName,            tx, shipList, config, players));

    // Empty ship list (=non-resolvable names)
    game::spec::ShipList emptySL;
    verifyNewNull   (a("ivsAuxName 0 empty"),   getVcrSideProperty(b, 0, game::interface::ivsAuxName,         tx, emptySL,  config, players));
    verifyNewNull   (a("ivsAuxShort 0 empty"),  getVcrSideProperty(b, 0, game::interface::ivsAuxShort,        tx, emptySL,  config, players));
    verifyNewNull   (a("ivsTorpName 2 empty"),  getVcrSideProperty(b, 2, game::interface::ivsTorpName,        tx, emptySL,  config, players));
    verifyNewNull   (a("ivsTorpShort 2 empty"), getVcrSideProperty(b, 2, game::interface::ivsTorpShort,       tx, emptySL,  config, players));
    verifyNewNull   (a("ivsBeamName 2 empty"),  getVcrSideProperty(b, 2, game::interface::ivsBeamName,        tx, emptySL,  config, players));
    verifyNewNull   (a("ivsBeamShort 2 empty"), getVcrSideProperty(b, 2, game::interface::ivsBeamShort,       tx, emptySL,  config, players));
}
