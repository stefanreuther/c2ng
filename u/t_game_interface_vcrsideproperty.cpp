/**
  *  \file u/t_game_interface_vcrsideproperty.cpp
  *  \brief Test for game::interface::VcrSideProperty
  */

#include "game/interface/vcrsideproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
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

void
TestGameInterfaceVcrSideProperty::testIt()
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
    verifyNewInteger("ivsAuxAmmo 0",         getVcrSideProperty(b, 0, game::interface::ivsAuxAmmo,         tx, shipList, config, players), 320);
    verifyNewInteger("ivsAuxCount 0",        getVcrSideProperty(b, 0, game::interface::ivsAuxCount,        tx, shipList, config, players), 7);
    verifyNewInteger("ivsAuxId 0",           getVcrSideProperty(b, 0, game::interface::ivsAuxId,           tx, shipList, config, players), 3);
    verifyNewString ("ivsAuxName 0",         getVcrSideProperty(b, 0, game::interface::ivsAuxName,         tx, shipList, config, players), "Mark 2 Photon");
    verifyNewString ("ivsAuxShort 0",        getVcrSideProperty(b, 0, game::interface::ivsAuxShort,        tx, shipList, config, players), "torp3");
    verifyNewInteger("ivsFighterBays 0",     getVcrSideProperty(b, 0, game::interface::ivsFighterBays,     tx, shipList, config, players), 0);
    verifyNewInteger("ivsFighterCount 0",    getVcrSideProperty(b, 0, game::interface::ivsFighterCount,    tx, shipList, config, players), 0);
    verifyNewInteger("ivsTorpId 0",          getVcrSideProperty(b, 0, game::interface::ivsTorpId,          tx, shipList, config, players), 3);
    verifyNewInteger("ivsTorpCount 0",       getVcrSideProperty(b, 0, game::interface::ivsTorpCount,       tx, shipList, config, players), 320);
    verifyNewInteger("ivsTorpLCount 0",      getVcrSideProperty(b, 0, game::interface::ivsTorpLCount,      tx, shipList, config, players), 7);
    verifyNewString ("ivsTorpShort 0",       getVcrSideProperty(b, 0, game::interface::ivsTorpShort,       tx, shipList, config, players), "torp3");
    verifyNewString ("ivsTorpName 0",        getVcrSideProperty(b, 0, game::interface::ivsTorpName,        tx, shipList, config, players), "Mark 2 Photon");
    verifyNewInteger("ivsBeamCount 0",       getVcrSideProperty(b, 0, game::interface::ivsBeamCount,       tx, shipList, config, players), 10);
    verifyNewInteger("ivsBeamId 0",          getVcrSideProperty(b, 0, game::interface::ivsBeamId,          tx, shipList, config, players), 5);
    verifyNewString ("ivsBeamName 0",        getVcrSideProperty(b, 0, game::interface::ivsBeamName,        tx, shipList, config, players), "Positron Beam");
    verifyNewString ("ivsBeamShort 0",       getVcrSideProperty(b, 0, game::interface::ivsBeamShort,       tx, shipList, config, players), "beam5");
    verifyNewInteger("ivsCrew 0",            getVcrSideProperty(b, 0, game::interface::ivsCrew,            tx, shipList, config, players), 500);
    verifyNewInteger("ivsCrewRaw 0",         getVcrSideProperty(b, 0, game::interface::ivsCrewRaw,         tx, shipList, config, players), 500);
    verifyNewInteger("ivsDamage 0",          getVcrSideProperty(b, 0, game::interface::ivsDamage,          tx, shipList, config, players), 2);
    verifyNewInteger("ivsId 0",              getVcrSideProperty(b, 0, game::interface::ivsId,              tx, shipList, config, players), 70);
    verifyNewInteger("ivsMass 0",            getVcrSideProperty(b, 0, game::interface::ivsMass,            tx, shipList, config, players), 2000);
    verifyNewString ("ivsName 0",            getVcrSideProperty(b, 0, game::interface::ivsName,            tx, shipList, config, players), "Anni");
    verifyNewString ("ivsNameFull 0",        getVcrSideProperty(b, 0, game::interface::ivsNameFull,        tx, shipList, config, players), "Anni (Ship #70)");
    verifyNewString ("ivsOwnerAdj 0",        getVcrSideProperty(b, 0, game::interface::ivsOwnerAdj,        tx, shipList, config, players), "Lizard");
    verifyNewInteger("ivsOwnerId 0",         getVcrSideProperty(b, 0, game::interface::ivsOwnerId,         tx, shipList, config, players), 2);
    verifyNewString ("ivsOwnerShort 0",      getVcrSideProperty(b, 0, game::interface::ivsOwnerShort,      tx, shipList, config, players), "The Lizards");
    verifyNewInteger("ivsShield 0",          getVcrSideProperty(b, 0, game::interface::ivsShield,          tx, shipList, config, players), 98);
    verifyNewString ("ivsStatus 0",          getVcrSideProperty(b, 0, game::interface::ivsStatus,          tx, shipList, config, players), "Captured");
    verifyNewInteger("ivsStatusRaw 0",       getVcrSideProperty(b, 0, game::interface::ivsStatusRaw,       tx, shipList, config, players), 3);
    verifyNewString ("ivsType 0",            getVcrSideProperty(b, 0, game::interface::ivsType,            tx, shipList, config, players), "Torpedo Ship");
    verifyNewString ("ivsTypeShort 0",       getVcrSideProperty(b, 0, game::interface::ivsTypeShort,       tx, shipList, config, players), "T");
    verifyNewString ("ivsHullName 0",        getVcrSideProperty(b, 0, game::interface::ivsHullName,        tx, shipList, config, players), "ANNIHILATION CLASS BATTLESHIP");
    verifyNewInteger("ivsHullId 0",          getVcrSideProperty(b, 0, game::interface::ivsHullId,          tx, shipList, config, players), game::test::ANNIHILATION_HULL_ID);
    verifyNewInteger("ivsImage 0",           getVcrSideProperty(b, 0, game::interface::ivsImage,           tx, shipList, config, players), 84);
    verifyNewInteger("ivsLevel 0",           getVcrSideProperty(b, 0, game::interface::ivsLevel,           tx, shipList, config, players), 1);
    verifyNewBoolean("ivsIsPlanet 0",        getVcrSideProperty(b, 0, game::interface::ivsIsPlanet,        tx, shipList, config, players), false);
    verifyNewInteger("ivsBeamKillRate 0",    getVcrSideProperty(b, 0, game::interface::ivsBeamKillRate,    tx, shipList, config, players), 3);
    verifyNewInteger("ivsBeamChargeRate 0",  getVcrSideProperty(b, 0, game::interface::ivsBeamChargeRate,  tx, shipList, config, players), 1);
    verifyNewInteger("ivsTorpMissRate 0",    getVcrSideProperty(b, 0, game::interface::ivsTorpMissRate,    tx, shipList, config, players), 40);
    verifyNewInteger("ivsTorpChargeRate 0",  getVcrSideProperty(b, 0, game::interface::ivsTorpChargeRate,  tx, shipList, config, players), 2);
    verifyNewInteger("ivsCrewDefenseRate 0", getVcrSideProperty(b, 0, game::interface::ivsCrewDefenseRate, tx, shipList, config, players), 10);
    verifyNewString ("ivsRole 0",            getVcrSideProperty(b, 0, game::interface::ivsRole,            tx, shipList, config, players), "aggressor");

    // Verify slot 1 (Gorbie)
    verifyNewInteger("ivsAuxAmmo 1",         getVcrSideProperty(b, 1, game::interface::ivsAuxAmmo,         tx, shipList, config, players), 180);
    verifyNewInteger("ivsAuxCount 1",        getVcrSideProperty(b, 1, game::interface::ivsAuxCount,        tx, shipList, config, players), 8);
    verifyNewInteger("ivsAuxId 1",           getVcrSideProperty(b, 1, game::interface::ivsAuxId,           tx, shipList, config, players), 11);
    verifyNewString ("ivsAuxName 1",         getVcrSideProperty(b, 1, game::interface::ivsAuxName,         tx, shipList, config, players), "Fighters");
    verifyNewString ("ivsAuxShort 1",        getVcrSideProperty(b, 1, game::interface::ivsAuxShort,        tx, shipList, config, players), "Ftr");
    verifyNewInteger("ivsFighterBays 1",     getVcrSideProperty(b, 1, game::interface::ivsFighterBays,     tx, shipList, config, players), 8);
    verifyNewInteger("ivsFighterCount 1",    getVcrSideProperty(b, 1, game::interface::ivsFighterCount,    tx, shipList, config, players), 180);
    verifyNewNull   ("ivsTorpId 1",          getVcrSideProperty(b, 1, game::interface::ivsTorpId,          tx, shipList, config, players));
    verifyNewInteger("ivsTorpCount 1",       getVcrSideProperty(b, 1, game::interface::ivsTorpCount,       tx, shipList, config, players), 0);
    verifyNewInteger("ivsTorpLCount 1",      getVcrSideProperty(b, 1, game::interface::ivsTorpLCount,      tx, shipList, config, players), 0);
    verifyNewNull   ("ivsTorpShort 1",       getVcrSideProperty(b, 1, game::interface::ivsTorpShort,       tx, shipList, config, players));
    verifyNewNull   ("ivsTorpName 1",        getVcrSideProperty(b, 1, game::interface::ivsTorpName,        tx, shipList, config, players));
    verifyNewInteger("ivsBeamCount 1",       getVcrSideProperty(b, 1, game::interface::ivsBeamCount,       tx, shipList, config, players), 0);
    verifyNewInteger("ivsBeamId 1",          getVcrSideProperty(b, 1, game::interface::ivsBeamId,          tx, shipList, config, players), 0);
    verifyNewNull   ("ivsBeamName 1",        getVcrSideProperty(b, 1, game::interface::ivsBeamName,        tx, shipList, config, players));
    verifyNewNull   ("ivsBeamShort 1",       getVcrSideProperty(b, 1, game::interface::ivsBeamShort,       tx, shipList, config, players));
    verifyNewInteger("ivsCrew 1",            getVcrSideProperty(b, 1, game::interface::ivsCrew,            tx, shipList, config, players), 700);
    verifyNewInteger("ivsCrewRaw 1",         getVcrSideProperty(b, 1, game::interface::ivsCrewRaw,         tx, shipList, config, players), 700);
    verifyNewInteger("ivsDamage 1",          getVcrSideProperty(b, 1, game::interface::ivsDamage,          tx, shipList, config, players), 0);
    verifyNewInteger("ivsId 1",              getVcrSideProperty(b, 1, game::interface::ivsId,              tx, shipList, config, players), 90);
    verifyNewInteger("ivsMass 1",            getVcrSideProperty(b, 1, game::interface::ivsMass,            tx, shipList, config, players), 1800);
    verifyNewString ("ivsName 1",            getVcrSideProperty(b, 1, game::interface::ivsName,            tx, shipList, config, players), "Michal");
    verifyNewString ("ivsNameFull 1",        getVcrSideProperty(b, 1, game::interface::ivsNameFull,        tx, shipList, config, players), "Michal (Ship #90)");
    verifyNewString ("ivsOwnerAdj 1",        getVcrSideProperty(b, 1, game::interface::ivsOwnerAdj,        tx, shipList, config, players), "Pirates");
    verifyNewInteger("ivsOwnerId 1",         getVcrSideProperty(b, 1, game::interface::ivsOwnerId,         tx, shipList, config, players), 5);
    verifyNewString ("ivsOwnerShort 1",      getVcrSideProperty(b, 1, game::interface::ivsOwnerShort,      tx, shipList, config, players), "The Pirates");
    verifyNewInteger("ivsShield 1",          getVcrSideProperty(b, 1, game::interface::ivsShield,          tx, shipList, config, players), 100);
    verifyNewString ("ivsStatus 1",          getVcrSideProperty(b, 1, game::interface::ivsStatus,          tx, shipList, config, players), "Exploded");
    verifyNewInteger("ivsStatusRaw 1",       getVcrSideProperty(b, 1, game::interface::ivsStatusRaw,       tx, shipList, config, players), -1);
    verifyNewString ("ivsType 1",            getVcrSideProperty(b, 1, game::interface::ivsType,            tx, shipList, config, players), "Carrier");
    verifyNewString ("ivsTypeShort 1",       getVcrSideProperty(b, 1, game::interface::ivsTypeShort,       tx, shipList, config, players), "C");
    verifyNewString ("ivsHullName 1",        getVcrSideProperty(b, 1, game::interface::ivsHullName,        tx, shipList, config, players), "GORBIE CLASS BATTLECARRIER");
    verifyNewInteger("ivsHullId 1",          getVcrSideProperty(b, 1, game::interface::ivsHullId,          tx, shipList, config, players), game::test::GORBIE_HULL_ID);
    verifyNewInteger("ivsImage 1",           getVcrSideProperty(b, 1, game::interface::ivsImage,           tx, shipList, config, players), 107);
    verifyNewInteger("ivsLevel 1",           getVcrSideProperty(b, 1, game::interface::ivsLevel,           tx, shipList, config, players), 0);
    verifyNewBoolean("ivsIsPlanet 1",        getVcrSideProperty(b, 1, game::interface::ivsIsPlanet,        tx, shipList, config, players), false);
    verifyNewInteger("ivsBeamKillRate 1",    getVcrSideProperty(b, 1, game::interface::ivsBeamKillRate,    tx, shipList, config, players), 1);
    verifyNewInteger("ivsBeamChargeRate 1",  getVcrSideProperty(b, 1, game::interface::ivsBeamChargeRate,  tx, shipList, config, players), 1);
    verifyNewInteger("ivsTorpMissRate 1",    getVcrSideProperty(b, 1, game::interface::ivsTorpMissRate,    tx, shipList, config, players), 10);
    verifyNewInteger("ivsTorpChargeRate 1",  getVcrSideProperty(b, 1, game::interface::ivsTorpChargeRate,  tx, shipList, config, players), 2);
    verifyNewInteger("ivsCrewDefenseRate 1", getVcrSideProperty(b, 1, game::interface::ivsCrewDefenseRate, tx, shipList, config, players), 10);
    verifyNewString ("ivsRole 1",            getVcrSideProperty(b, 1, game::interface::ivsRole,            tx, shipList, config, players), "opponent");

    // Verify slot 2 (planet)
    verifyNewInteger("ivsAuxAmmo 2",         getVcrSideProperty(b, 2, game::interface::ivsAuxAmmo,         tx, shipList, config, players), 10);
    verifyNewInteger("ivsAuxCount 2",        getVcrSideProperty(b, 2, game::interface::ivsAuxCount,        tx, shipList, config, players), 5);
    verifyNewInteger("ivsAuxId 2",           getVcrSideProperty(b, 2, game::interface::ivsAuxId,           tx, shipList, config, players), 11);
    verifyNewString ("ivsAuxName 2",         getVcrSideProperty(b, 2, game::interface::ivsAuxName,         tx, shipList, config, players), "Fighters");
    verifyNewString ("ivsAuxShort 2",        getVcrSideProperty(b, 2, game::interface::ivsAuxShort,        tx, shipList, config, players), "Ftr");
    verifyNewInteger("ivsFighterBays 2",     getVcrSideProperty(b, 2, game::interface::ivsFighterBays,     tx, shipList, config, players), 5);
    verifyNewInteger("ivsFighterCount 2",    getVcrSideProperty(b, 2, game::interface::ivsFighterCount,    tx, shipList, config, players), 10);
    verifyNewInteger("ivsTorpId 2",          getVcrSideProperty(b, 2, game::interface::ivsTorpId,          tx, shipList, config, players), 2);
    verifyNewInteger("ivsTorpCount 2",       getVcrSideProperty(b, 2, game::interface::ivsTorpCount,       tx, shipList, config, players), 12);
    verifyNewInteger("ivsTorpLCount 2",      getVcrSideProperty(b, 2, game::interface::ivsTorpLCount,      tx, shipList, config, players), 2);
    verifyNewString ("ivsTorpShort 2",       getVcrSideProperty(b, 2, game::interface::ivsTorpShort,       tx, shipList, config, players), "torp2");
    verifyNewString ("ivsTorpName 2",        getVcrSideProperty(b, 2, game::interface::ivsTorpName,        tx, shipList, config, players), "Proton torp");
    verifyNewInteger("ivsBeamCount 2",       getVcrSideProperty(b, 2, game::interface::ivsBeamCount,       tx, shipList, config, players), 4);
    verifyNewInteger("ivsBeamId 2",          getVcrSideProperty(b, 2, game::interface::ivsBeamId,          tx, shipList, config, players), 4);
    verifyNewString ("ivsBeamName 2",        getVcrSideProperty(b, 2, game::interface::ivsBeamName,        tx, shipList, config, players), "Blaster");
    verifyNewString ("ivsBeamShort 2",       getVcrSideProperty(b, 2, game::interface::ivsBeamShort,       tx, shipList, config, players), "beam4");
    verifyNewNull   ("ivsCrew 2",            getVcrSideProperty(b, 2, game::interface::ivsCrew,            tx, shipList, config, players));
    verifyNewInteger("ivsCrewRaw 2",         getVcrSideProperty(b, 2, game::interface::ivsCrewRaw,         tx, shipList, config, players), 0);
    verifyNewInteger("ivsDamage 2",          getVcrSideProperty(b, 2, game::interface::ivsDamage,          tx, shipList, config, players), 0);
    verifyNewInteger("ivsId 2",              getVcrSideProperty(b, 2, game::interface::ivsId,              tx, shipList, config, players), 363);
    verifyNewInteger("ivsMass 2",            getVcrSideProperty(b, 2, game::interface::ivsMass,            tx, shipList, config, players), 120);
    verifyNewString ("ivsName 2",            getVcrSideProperty(b, 2, game::interface::ivsName,            tx, shipList, config, players), "Melmac");
    verifyNewString ("ivsNameFull 2",        getVcrSideProperty(b, 2, game::interface::ivsNameFull,        tx, shipList, config, players), "Melmac (Planet #363)");
    verifyNewString ("ivsOwnerAdj 2",        getVcrSideProperty(b, 2, game::interface::ivsOwnerAdj,        tx, shipList, config, players), "Pirates");
    verifyNewInteger("ivsOwnerId 2",         getVcrSideProperty(b, 2, game::interface::ivsOwnerId,         tx, shipList, config, players), 5);
    verifyNewString ("ivsOwnerShort 2",      getVcrSideProperty(b, 2, game::interface::ivsOwnerShort,      tx, shipList, config, players), "The Pirates");
    verifyNewInteger("ivsShield 2",          getVcrSideProperty(b, 2, game::interface::ivsShield,          tx, shipList, config, players), 100);
    verifyNewString ("ivsStatus 2",          getVcrSideProperty(b, 2, game::interface::ivsStatus,          tx, shipList, config, players), "Survived");
    verifyNewInteger("ivsStatusRaw 2",       getVcrSideProperty(b, 2, game::interface::ivsStatusRaw,       tx, shipList, config, players), 0);
    verifyNewString ("ivsType 2",            getVcrSideProperty(b, 2, game::interface::ivsType,            tx, shipList, config, players), "Planet");
    verifyNewString ("ivsTypeShort 2",       getVcrSideProperty(b, 2, game::interface::ivsTypeShort,       tx, shipList, config, players), "P");
    verifyNewNull   ("ivsHullName 2",        getVcrSideProperty(b, 2, game::interface::ivsHullName,        tx, shipList, config, players));
    verifyNewNull   ("ivsHullId 2",          getVcrSideProperty(b, 2, game::interface::ivsHullId,          tx, shipList, config, players));
    verifyNewInteger("ivsImage 2",           getVcrSideProperty(b, 2, game::interface::ivsImage,           tx, shipList, config, players), 0);
    verifyNewInteger("ivsLevel 2",           getVcrSideProperty(b, 2, game::interface::ivsLevel,           tx, shipList, config, players), 0);
    verifyNewBoolean("ivsIsPlanet 2",        getVcrSideProperty(b, 2, game::interface::ivsIsPlanet,        tx, shipList, config, players), true);
    verifyNewInteger("ivsBeamKillRate 2",    getVcrSideProperty(b, 2, game::interface::ivsBeamKillRate,    tx, shipList, config, players), 1);
    verifyNewInteger("ivsBeamChargeRate 2",  getVcrSideProperty(b, 2, game::interface::ivsBeamChargeRate,  tx, shipList, config, players), 1);
    verifyNewInteger("ivsTorpMissRate 2",    getVcrSideProperty(b, 2, game::interface::ivsTorpMissRate,    tx, shipList, config, players), 10);
    verifyNewInteger("ivsTorpChargeRate 2",  getVcrSideProperty(b, 2, game::interface::ivsTorpChargeRate,  tx, shipList, config, players), 2);
    verifyNewInteger("ivsCrewDefenseRate 2", getVcrSideProperty(b, 2, game::interface::ivsCrewDefenseRate, tx, shipList, config, players), 10);
    verifyNewNull   ("ivsRole 2",            getVcrSideProperty(b, 2, game::interface::ivsRole,            tx, shipList, config, players));

    // Verify slot 3 (freigher)
    verifyNewInteger("ivsAuxAmmo 3",         getVcrSideProperty(b, 3, game::interface::ivsAuxAmmo,         tx, shipList, config, players), 0);
    verifyNewNull   ("ivsAuxCount 3",        getVcrSideProperty(b, 3, game::interface::ivsAuxCount,        tx, shipList, config, players));
    verifyNewNull   ("ivsAuxId 3",           getVcrSideProperty(b, 3, game::interface::ivsAuxId,           tx, shipList, config, players));
    verifyNewNull   ("ivsAuxName 3",         getVcrSideProperty(b, 3, game::interface::ivsAuxName,         tx, shipList, config, players));
    verifyNewNull   ("ivsAuxShort 3",        getVcrSideProperty(b, 3, game::interface::ivsAuxShort,        tx, shipList, config, players));
    verifyNewInteger("ivsFighterBays 3",     getVcrSideProperty(b, 3, game::interface::ivsFighterBays,     tx, shipList, config, players), 0);
    verifyNewInteger("ivsFighterCount 3",    getVcrSideProperty(b, 3, game::interface::ivsFighterCount,    tx, shipList, config, players), 0);
    verifyNewNull   ("ivsTorpId 3",          getVcrSideProperty(b, 3, game::interface::ivsTorpId,          tx, shipList, config, players));
    verifyNewInteger("ivsTorpCount 3",       getVcrSideProperty(b, 3, game::interface::ivsTorpCount,       tx, shipList, config, players), 0);
    verifyNewInteger("ivsTorpLCount 3",      getVcrSideProperty(b, 3, game::interface::ivsTorpLCount,      tx, shipList, config, players), 0);
    verifyNewNull   ("ivsTorpShort 3",       getVcrSideProperty(b, 3, game::interface::ivsTorpShort,       tx, shipList, config, players));
    verifyNewNull   ("ivsTorpName 3",        getVcrSideProperty(b, 3, game::interface::ivsTorpName,        tx, shipList, config, players));
    verifyNewInteger("ivsBeamCount 3",       getVcrSideProperty(b, 3, game::interface::ivsBeamCount,       tx, shipList, config, players), 0);
    verifyNewInteger("ivsBeamId 3",          getVcrSideProperty(b, 3, game::interface::ivsBeamId,          tx, shipList, config, players), 0);
    verifyNewNull   ("ivsBeamName 3",        getVcrSideProperty(b, 3, game::interface::ivsBeamName,        tx, shipList, config, players));
    verifyNewNull   ("ivsBeamShort 3",       getVcrSideProperty(b, 3, game::interface::ivsBeamShort,       tx, shipList, config, players));
    verifyNewInteger("ivsCrew 3",            getVcrSideProperty(b, 3, game::interface::ivsCrew,            tx, shipList, config, players), 10);
    verifyNewInteger("ivsCrewRaw 3",         getVcrSideProperty(b, 3, game::interface::ivsCrewRaw,         tx, shipList, config, players), 10);
    verifyNewInteger("ivsDamage 3",          getVcrSideProperty(b, 3, game::interface::ivsDamage,          tx, shipList, config, players), 0);
    verifyNewInteger("ivsId 3",              getVcrSideProperty(b, 3, game::interface::ivsId,              tx, shipList, config, players), 150);
    verifyNewInteger("ivsMass 3",            getVcrSideProperty(b, 3, game::interface::ivsMass,            tx, shipList, config, players), 20);
    verifyNewString ("ivsName 3",            getVcrSideProperty(b, 3, game::interface::ivsName,            tx, shipList, config, players), "Cargo");
    verifyNewString ("ivsNameFull 3",        getVcrSideProperty(b, 3, game::interface::ivsNameFull,        tx, shipList, config, players), "Cargo (Ship #150)");
    verifyNewString ("ivsOwnerAdj 3",        getVcrSideProperty(b, 3, game::interface::ivsOwnerAdj,        tx, shipList, config, players), "Pirates");
    verifyNewInteger("ivsOwnerId 3",         getVcrSideProperty(b, 3, game::interface::ivsOwnerId,         tx, shipList, config, players), 5);
    verifyNewString ("ivsOwnerShort 3",      getVcrSideProperty(b, 3, game::interface::ivsOwnerShort,      tx, shipList, config, players), "The Pirates");
    verifyNewInteger("ivsShield 3",          getVcrSideProperty(b, 3, game::interface::ivsShield,          tx, shipList, config, players), 0);
    verifyNewString ("ivsStatus 3",          getVcrSideProperty(b, 3, game::interface::ivsStatus,          tx, shipList, config, players), "Survived");
    verifyNewInteger("ivsStatusRaw 3",       getVcrSideProperty(b, 3, game::interface::ivsStatusRaw,       tx, shipList, config, players), 0);
    verifyNewString ("ivsType 3",            getVcrSideProperty(b, 3, game::interface::ivsType,            tx, shipList, config, players), "Freighter");
    verifyNewString ("ivsTypeShort 3",       getVcrSideProperty(b, 3, game::interface::ivsTypeShort,       tx, shipList, config, players), "F");
    verifyNewNull   ("ivsHullName 3",        getVcrSideProperty(b, 3, game::interface::ivsHullName,        tx, shipList, config, players));
    verifyNewNull   ("ivsHullId 3",          getVcrSideProperty(b, 3, game::interface::ivsHullId,          tx, shipList, config, players));
    verifyNewInteger("ivsImage 3",           getVcrSideProperty(b, 3, game::interface::ivsImage,           tx, shipList, config, players), 10);
    verifyNewInteger("ivsLevel 3",           getVcrSideProperty(b, 3, game::interface::ivsLevel,           tx, shipList, config, players), 0);
    verifyNewBoolean("ivsIsPlanet 3",        getVcrSideProperty(b, 3, game::interface::ivsIsPlanet,        tx, shipList, config, players), false);
    verifyNewInteger("ivsBeamKillRate 3",    getVcrSideProperty(b, 3, game::interface::ivsBeamKillRate,    tx, shipList, config, players), 1);
    verifyNewInteger("ivsBeamChargeRate 3",  getVcrSideProperty(b, 3, game::interface::ivsBeamChargeRate,  tx, shipList, config, players), 1);
    verifyNewInteger("ivsTorpMissRate 3",    getVcrSideProperty(b, 3, game::interface::ivsTorpMissRate,    tx, shipList, config, players), 10);
    verifyNewInteger("ivsTorpChargeRate 3",  getVcrSideProperty(b, 3, game::interface::ivsTorpChargeRate,  tx, shipList, config, players), 2);
    verifyNewInteger("ivsCrewDefenseRate 3", getVcrSideProperty(b, 3, game::interface::ivsCrewDefenseRate, tx, shipList, config, players), 10);
    verifyNewNull   ("ivsRole 3",            getVcrSideProperty(b, 3, game::interface::ivsRole,            tx, shipList, config, players));

    // Out-of-range
    verifyNewNull   ("ivsName 4",            getVcrSideProperty(b, 4, game::interface::ivsName,            tx, shipList, config, players));

    // Empty ship list (=non-resolvable names)
    game::spec::ShipList emptySL;
    verifyNewNull   ("ivsAuxName 0 empty",   getVcrSideProperty(b, 0, game::interface::ivsAuxName,         tx, emptySL,  config, players));
    verifyNewNull   ("ivsAuxShort 0 empty",  getVcrSideProperty(b, 0, game::interface::ivsAuxShort,        tx, emptySL,  config, players));
    verifyNewNull   ("ivsTorpName 2 empty",  getVcrSideProperty(b, 2, game::interface::ivsTorpName,        tx, emptySL,  config, players));
    verifyNewNull   ("ivsTorpShort 2 empty", getVcrSideProperty(b, 2, game::interface::ivsTorpShort,       tx, emptySL,  config, players));
    verifyNewNull   ("ivsBeamName 2 empty",  getVcrSideProperty(b, 2, game::interface::ivsBeamName,        tx, emptySL,  config, players));
    verifyNewNull   ("ivsBeamShort 2 empty", getVcrSideProperty(b, 2, game::interface::ivsBeamShort,       tx, emptySL,  config, players));
}

