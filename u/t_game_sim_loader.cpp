/**
  *  \file u/t_game_sim_loader.cpp
  *  \brief Test for game::sim::Loader
  *
  *  These test cases use actual .ccb files created by PCC/CCBSim/PCC2/PlayVCR.
  */

#include "game/sim/loader.hpp"

#include "t_game_sim.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "u/files.hpp"

namespace {
    /* Prepare a setup for the "save" tests */
    void prepare(game::sim::Setup& setup)
    {
        game::sim::Ship* sh = setup.addShip();
        sh->setName("Boat");
        sh->setOwner(3);
        sh->setTorpedoType(7);
        sh->setNumLaunchers(4);
        sh->setAmmo(180);
        sh->setCrew(17);
        sh->setFriendlyCode("abc");
        sh->setEngineType(8);

        game::sim::Planet* pl = setup.addPlanet();
        pl->setOwner(4);
        pl->setDefense(61);
        pl->setFriendlyCode("xyz");
        pl->setBaseBeamTech(6);
        pl->setBaseTorpedoTech(9);
        pl->setBaseDefense(12);
    }
}

/** Test V0 file format (PCC 0.99.10). */
void
TestGameSimLoader::testV0()
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);
    afl::io::ConstMemoryStream stream(getSimFileV0());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    TS_ASSERT_EQUALS(result.getNumShips(), 2U);
    TS_ASSERT_EQUALS(result.getNumObjects(), 3U);
    TS_ASSERT(result.hasPlanet());

    // First ship
    game::sim::Ship* sh = result.getShip(0);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "C.C.S.S. Joker");
    TS_ASSERT_EQUALS(sh->getHullType(), 61);  // Emerald
    TS_ASSERT_EQUALS(sh->getOwner(), 7);
    TS_ASSERT_EQUALS(sh->getId(), 117);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "NTP");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 258);
    // TS_ASSERT_EQUALS(sh->getMass(), 180); // not set, needs ship list
    TS_ASSERT_EQUALS(sh->getNumBeams(), 8);
    TS_ASSERT_EQUALS(sh->getBeamType(), 7);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 3);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 10);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 40);
    TS_ASSERT_EQUALS(sh->getEngineType(), 7);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    TS_ASSERT_EQUALS(sh->getFlags(), 0);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);

    // Second ship
    sh = result.getShip(1);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "C.C.S.S. Claudrin II");
    TS_ASSERT_EQUALS(sh->getHullType(), 22);  // LCC
    TS_ASSERT_EQUALS(sh->getOwner(), 7);
    TS_ASSERT_EQUALS(sh->getId(), 9);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "NTP");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 430);
    TS_ASSERT_EQUALS(sh->getNumBeams(), 4);
    TS_ASSERT_EQUALS(sh->getBeamType(), 6);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 3);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 6);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 50);
    TS_ASSERT_EQUALS(sh->getEngineType(), 9);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    TS_ASSERT_EQUALS(sh->getFlags(), 0);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);

    // Planet
    game::sim::Planet* pl = result.getPlanet();
    TS_ASSERT(pl);
    TS_ASSERT_EQUALS(pl->getId(), 1);
    TS_ASSERT_EQUALS(pl->getOwner(), 2);
    TS_ASSERT_EQUALS(pl->getFriendlyCode(), "i9m");
    TS_ASSERT_EQUALS(pl->getDefense(), 62);
    TS_ASSERT_EQUALS(pl->getFlags(), 0);
    TS_ASSERT_EQUALS(pl->getExperienceLevel(), 0);
    TS_ASSERT_EQUALS(pl->getBaseBeamTech(), 0);
    // TS_ASSERT_EQUALS(pl->getNumBaseFighters(), 0); // not set
    // TS_ASSERT_EQUALS(pl->getBaseDefense(), 0);     // not set
    // TS_ASSERT_EQUALS(pl->getBaseTorpedoTech(), 0); // not set
}

/** Test V1 file format (PCC 1.0). */
void
TestGameSimLoader::testV1()
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);
    afl::io::ConstMemoryStream stream(getSimFileV1());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    TS_ASSERT_EQUALS(result.getNumShips(), 1U);
    TS_ASSERT_EQUALS(result.getNumObjects(), 2U);
    TS_ASSERT(result.hasPlanet());

    // Ship
    game::sim::Ship* sh = result.getShip(0);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "Ship 201");
    TS_ASSERT_EQUALS(sh->getHullType(), 76);  // SSC
    TS_ASSERT_EQUALS(sh->getOwner(), 8);
    TS_ASSERT_EQUALS(sh->getId(), 201);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "?""?""?");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 352);
    // TS_ASSERT_EQUALS(sh->getMass(), 180); // not set, needs ship list
    TS_ASSERT_EQUALS(sh->getNumBeams(), 6);
    TS_ASSERT_EQUALS(sh->getBeamType(), 6);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 0);
    TS_ASSERT_EQUALS(sh->getNumBays(), 4);
    TS_ASSERT_EQUALS(sh->getAmmo(), 85);
    TS_ASSERT_EQUALS(sh->getEngineType(), 9);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    TS_ASSERT_EQUALS(sh->getFlags(), 0);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);

    // Planet
    game::sim::Planet* pl = result.getPlanet();
    TS_ASSERT(pl);
    TS_ASSERT_EQUALS(pl->getId(), 459);
    TS_ASSERT_EQUALS(pl->getOwner(), 6);
    TS_ASSERT_EQUALS(pl->getFriendlyCode(), "NUK");
    TS_ASSERT_EQUALS(pl->getDefense(), 129);
    TS_ASSERT_EQUALS(pl->getFlags(), 0);
    TS_ASSERT_EQUALS(pl->getExperienceLevel(), 0);
    TS_ASSERT_EQUALS(pl->getBaseBeamTech(), 1);
    TS_ASSERT_EQUALS(pl->getNumBaseFighters(), 22);
    TS_ASSERT_EQUALS(pl->getBaseDefense(), 150);
    TS_ASSERT_EQUALS(pl->getBaseTorpedoTech(), 1);
}

/** Test V2 file format (PCC 1.0.7). */
void
TestGameSimLoader::testV2()
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);
    afl::io::ConstMemoryStream stream(getSimFileV2());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    TS_ASSERT_EQUALS(result.getNumShips(), 2U);
    TS_ASSERT_EQUALS(result.getNumObjects(), 3U);
    TS_ASSERT(result.hasPlanet());

    // First ship
    game::sim::Ship* sh = result.getShip(0);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "Ship 4");
    TS_ASSERT_EQUALS(sh->getHullType(), 1);  // Outrider
    TS_ASSERT_EQUALS(sh->getOwner(), 12);
    TS_ASSERT_EQUALS(sh->getId(), 4);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "?""?""?");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 180);
    // TS_ASSERT_EQUALS(sh->getMass(), 75); // not set, needs ship list
    TS_ASSERT_EQUALS(sh->getNumBeams(), 1);
    TS_ASSERT_EQUALS(sh->getBeamType(), 10);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 0);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 0);
    TS_ASSERT_EQUALS(sh->getEngineType(), 9);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_NoFuel);
    TS_ASSERT_EQUALS(sh->getFlags(), 0);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);

    // Second ship
    sh = result.getShip(1);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "Ship 5");
    TS_ASSERT_EQUALS(sh->getHullType(), 73);  // Mig Scout
    TS_ASSERT_EQUALS(sh->getOwner(), 8);
    TS_ASSERT_EQUALS(sh->getId(), 5);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "123");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 10);
    TS_ASSERT_EQUALS(sh->getNumBeams(), 2);
    TS_ASSERT_EQUALS(sh->getBeamType(), 10);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 0);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 0);
    TS_ASSERT_EQUALS(sh->getEngineType(), 9);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    TS_ASSERT_EQUALS(sh->getFlags(), 0);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);

    // Planet
    game::sim::Planet* pl = result.getPlanet();
    TS_ASSERT(pl);
    TS_ASSERT_EQUALS(pl->getId(), 1);
    TS_ASSERT_EQUALS(pl->getOwner(), 12);
    TS_ASSERT_EQUALS(pl->getFriendlyCode(), "NUK");
    TS_ASSERT_EQUALS(pl->getDefense(), 10);
    TS_ASSERT_EQUALS(pl->getFlags(), 0);
    TS_ASSERT_EQUALS(pl->getExperienceLevel(), 0);
    TS_ASSERT_EQUALS(pl->getBaseBeamTech(), 0);
    // TS_ASSERT_EQUALS(pl->getNumBaseFighters(), 0); // not set
    // TS_ASSERT_EQUALS(pl->getBaseDefense(), 0);     // not set
    // TS_ASSERT_EQUALS(pl->getBaseTorpedoTech(), 0); // not set
}

/** Test V3 file format (PCC 1.0.16, PCC2 1.99.2). */
void
TestGameSimLoader::testV3()
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);
    afl::io::ConstMemoryStream stream(getSimFileV3());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    TS_ASSERT_EQUALS(result.getNumShips(), 3U);
    TS_ASSERT_EQUALS(result.getNumObjects(), 4U);
    TS_ASSERT(result.hasPlanet());

    // First ship
    game::sim::Ship* sh = result.getShip(0);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "Ultra Elite Alien");
    TS_ASSERT_EQUALS(sh->getHullType(), 1);  // Outrider
    TS_ASSERT_EQUALS(sh->getOwner(), 12);
    TS_ASSERT_EQUALS(sh->getId(), 1);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "?""?""?");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 58);
    // TS_ASSERT_EQUALS(sh->getMass(), 75); // not set, needs ship list
    TS_ASSERT_EQUALS(sh->getNumBeams(), 1);
    TS_ASSERT_EQUALS(sh->getBeamType(), 10);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 0);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 0);
    TS_ASSERT_EQUALS(sh->getEngineType(), 9);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    TS_ASSERT_EQUALS(sh->getFlags(), game::sim::Object::fl_CommanderSet + game::sim::Object::fl_Commander);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 4);

    // Second ship
    sh = result.getShip(1);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "Recruit Alien");
    TS_ASSERT_EQUALS(sh->getHullType(), 1);  // Outrider
    TS_ASSERT_EQUALS(sh->getOwner(), 12);
    TS_ASSERT_EQUALS(sh->getId(), 2);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "?""?""?");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 58);
    // TS_ASSERT_EQUALS(sh->getMass(), 75); // not set, needs ship list
    TS_ASSERT_EQUALS(sh->getNumBeams(), 1);
    TS_ASSERT_EQUALS(sh->getBeamType(), 10);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 0);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 0);
    TS_ASSERT_EQUALS(sh->getEngineType(), 9);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    TS_ASSERT_EQUALS(sh->getFlags(), 0);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);

    // Third ship
    sh = result.getShip(2);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "Recruit Borg");
    TS_ASSERT_EQUALS(sh->getHullType(), 58);  // Quietus
    TS_ASSERT_EQUALS(sh->getOwner(), 6);
    TS_ASSERT_EQUALS(sh->getId(), 3);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "?""?""?");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 517);
    TS_ASSERT_EQUALS(sh->getNumBeams(), 9);
    TS_ASSERT_EQUALS(sh->getBeamType(), 10);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 9);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 10);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 260);
    TS_ASSERT_EQUALS(sh->getEngineType(), 9);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    TS_ASSERT_EQUALS(sh->getFlags(), 0);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);

    // Planet
    game::sim::Planet* pl = result.getPlanet();
    TS_ASSERT(pl);
    TS_ASSERT_EQUALS(pl->getId(), 1);
    TS_ASSERT_EQUALS(pl->getOwner(), 12);
    TS_ASSERT_EQUALS(pl->getFriendlyCode(), "?""?""?");
    TS_ASSERT_EQUALS(pl->getDefense(), 10);
    TS_ASSERT_EQUALS(pl->getFlags(), 0);
    TS_ASSERT_EQUALS(pl->getExperienceLevel(), 0);
    TS_ASSERT_EQUALS(pl->getBaseBeamTech(), 0);
}

/** Test V4 file format (PCC 1.1.11.6, PCC2 1.99.2). */
void
TestGameSimLoader::testV4()
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);
    afl::io::ConstMemoryStream stream(getSimFileV4());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    TS_ASSERT_EQUALS(result.getNumShips(), 1U);
    TS_ASSERT_EQUALS(result.getNumObjects(), 1U);
    TS_ASSERT(!result.hasPlanet());

    // The ship
    game::sim::Ship* sh = result.getShip(0);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "Ship 1");
    TS_ASSERT_EQUALS(sh->getHullType(), 1);  // Outrider
    TS_ASSERT_EQUALS(sh->getOwner(), 12);
    TS_ASSERT_EQUALS(sh->getId(), 1);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "?""?""?");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 58);
    // TS_ASSERT_EQUALS(sh->getMass(), 75); // not set, needs ship list
    TS_ASSERT_EQUALS(sh->getNumBeams(), 1);
    TS_ASSERT_EQUALS(sh->getBeamType(), 10);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 0);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 0);
    TS_ASSERT_EQUALS(sh->getEngineType(), 9);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Kill);
    TS_ASSERT_EQUALS(sh->getFlags(), game::sim::Ship::fl_RatingOverride);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);
    TS_ASSERT_EQUALS(sh->getFlakRatingOverride(), 240);
    TS_ASSERT_EQUALS(sh->getFlakCompensationOverride(), 23);
}

/** Test V5 file format (PCC2 1.99.22). */
void
TestGameSimLoader::testV5()
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);
    afl::io::ConstMemoryStream stream(getSimFileV5());

    game::sim::Setup result;
    testee.load(stream, result);

    // Basic properties
    TS_ASSERT_EQUALS(result.getNumShips(), 2U);
    TS_ASSERT_EQUALS(result.getNumObjects(), 2U);
    TS_ASSERT(!result.hasPlanet());

    // First ship
    game::sim::Ship* sh = result.getShip(0);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "Mike Oldfield");
    TS_ASSERT_EQUALS(sh->getHullType(), 16);  // MDSF
    TS_ASSERT_EQUALS(sh->getOwner(), 9);
    TS_ASSERT_EQUALS(sh->getId(), 1);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "_{=");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 6);
    TS_ASSERT_EQUALS(sh->getNumBeams(), 0);
    TS_ASSERT_EQUALS(sh->getBeamType(), 0);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 0);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 0);
    TS_ASSERT_EQUALS(sh->getEngineType(), 8);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Passive);
    TS_ASSERT_EQUALS(sh->getFlags(), 0);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);

    // Second ship
    sh = result.getShip(1);
    TS_ASSERT(sh);
    TS_ASSERT_EQUALS(sh->getName(), "Ma Baker");
    TS_ASSERT_EQUALS(sh->getHullType(), 17);  // LDSF
    TS_ASSERT_EQUALS(sh->getOwner(), 9);
    TS_ASSERT_EQUALS(sh->getId(), 6);
    TS_ASSERT_EQUALS(sh->getFriendlyCode(), "4R{");
    TS_ASSERT_EQUALS(sh->getDamage(), 0);
    TS_ASSERT_EQUALS(sh->getCrew(), 102);
    TS_ASSERT_EQUALS(sh->getNumBeams(), 0);
    TS_ASSERT_EQUALS(sh->getBeamType(), 0);
    TS_ASSERT_EQUALS(sh->getNumLaunchers(), 0);
    TS_ASSERT_EQUALS(sh->getTorpedoType(), 0);
    TS_ASSERT_EQUALS(sh->getNumBays(), 0);
    TS_ASSERT_EQUALS(sh->getAmmo(), 0);
    TS_ASSERT_EQUALS(sh->getEngineType(), 9);
    TS_ASSERT_EQUALS(sh->getAggressiveness(), game::sim::Ship::agg_Passive);
    TS_ASSERT_EQUALS(sh->getFlags(), game::sim::Object::fl_Elusive + game::sim::Object::fl_ElusiveSet);
    TS_ASSERT_EQUALS(sh->getInterceptId(), 0);
    TS_ASSERT_EQUALS(sh->getExperienceLevel(), 0);
}

/** Test error behaviour. */
void
TestGameSimLoader::testError()
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);
    game::sim::Setup result;

    // v0
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x73, 0x69, 0x6d, 0x1a, 0x02, 0x80, 0x43, 0x2e, 0x43, 0x2e,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // v1
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x1a, 0x01, 0x80, 0x53, 0x68,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // v2
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x31, 0x1a, 0x02, 0x80, 0x53, 0x68,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // v3
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x32, 0x1a, 0x03, 0x80, 0x55, 0x6c,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // v4
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x33, 0x1a, 0x01, 0x00, 0x53, 0x68,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // v5
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x34, 0x1a, 0x02, 0x00, 0x4d, 0x69,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // truncated signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x34,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // future signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x39, 0x1a,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // bad signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x00,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // bad signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43,
        };
        afl::io::ConstMemoryStream stream(FILE);
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }

    // empty file
    {
        afl::io::ConstMemoryStream stream((afl::base::ConstBytes_t()));
        TS_ASSERT_THROWS(testee.load(stream, result), afl::except::FileFormatException);
    }
}

/** Test saving in default format (V3). */
void
TestGameSimLoader::testSaveDefault()
{
    game::sim::Setup setup;
    prepare(setup);

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);

    afl::io::InternalStream stream;
    testee.save(stream, setup);

    static const uint8_t EXPECTED[] = {
        // Header (10)
        'C','C','b','s','i','m','2',26,1,0x80,

        // Ship (57)
        'B','o','a','t',32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,    // 20
        0,0, 17,0, 1,0, 3,0, 0,0, 0,0, 0,0, 0,0, 7,0, 180,0, 4,0,           // 22
        8,0, 0,0, 100,0, 'a','b','c', 0,0, 100,0, 0,0,

        // Planet (57)
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,                   // 20
        0,0, 0,0, 1,0, 4,0, 0,0, 6,0, 0,0, 0,0, 0,0, 0,0, 9,0,              // 22
        12,0, 61,0, 100,0, 'x','y','z', 0xFF,0xFF, 0,0, 0,0,
        // Note::   ^^^^^=shield,       ^^^^^^^^^=aggressiveness; this is not contractual
    };

    TS_ASSERT_EQUALS(stream.getContent().size(), sizeof(EXPECTED));
    TS_ASSERT_SAME_DATA(stream.getContent().at(0), EXPECTED, sizeof(EXPECTED));
}

/** Test saving with rating override (produces V4 format). */
void
TestGameSimLoader::testSaveRating()
{
    game::sim::Setup setup;
    prepare(setup);
    setup.getShip(0)->setFlakRatingOverride(99);
    setup.getShip(0)->setFlags(game::sim::Ship::fl_RatingOverride);

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);

    afl::io::InternalStream stream;
    testee.save(stream, setup);

    static const uint8_t EXPECTED[] = {
        // Header (10)
        'C','C','b','s','i','m','3',26,1,0x80,

        // Ship (65)
        'B','o','a','t',32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,    // 20
        0,0, 17,0, 1,0, 3,0, 0,0, 0,0, 0,0, 0,0, 7,0, 180,0, 4,0,           // 22
        8,0, 0,0, 100,0, 'a','b','c', 0,0, 100,0, 16,0,
        99,0,0,0, 0,0, 0,0,

        // Planet (65)
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,                   // 20
        0,0, 0,0, 1,0, 4,0, 0,0, 6,0, 0,0, 0,0, 0,0, 0,0, 9,0,              // 22
        12,0, 61,0, 100,0, 'x','y','z', 0xFF,0xFF, 0,0, 0,0,
        0,0,0,0, 0,0, 0,0,
    };

    TS_ASSERT_EQUALS(stream.getContent().size(), sizeof(EXPECTED));
    TS_ASSERT_SAME_DATA(stream.getContent().at(0), EXPECTED, sizeof(EXPECTED));
}

/** Test saving with intercept order (produces V4 format). */
void
TestGameSimLoader::testSaveIntercept()
{
    game::sim::Setup setup;
    prepare(setup);
    setup.getShip(0)->setInterceptId(12);

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);

    afl::io::InternalStream stream;
    testee.save(stream, setup);

    static const uint8_t EXPECTED[] = {
        // Header (10)
        'C','C','b','s','i','m','3',26,1,0x80,

        // Ship (65)
        'B','o','a','t',32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,    // 20
        0,0, 17,0, 1,0, 3,0, 0,0, 0,0, 0,0, 0,0, 7,0, 180,0, 4,0,           // 22
        8,0, 0,0, 100,0, 'a','b','c', 0,0, 100,0, 0,0,
        0,0,0,0, 0,0, 12,0,

        // Planet (65)
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,                   // 20
        0,0, 0,0, 1,0, 4,0, 0,0, 6,0, 0,0, 0,0, 0,0, 0,0, 9,0,              // 22
        12,0, 61,0, 100,0, 'x','y','z', 0xFF,0xFF, 0,0, 0,0,
        0,0,0,0, 0,0, 0,0,
    };

    TS_ASSERT_EQUALS(stream.getContent().size(), sizeof(EXPECTED));
    TS_ASSERT_SAME_DATA(stream.getContent().at(0), EXPECTED, sizeof(EXPECTED));
}

/** Test saving with long flags (produces V5 format). */
void
TestGameSimLoader::testSaveFlags()
{
    game::sim::Setup setup;
    prepare(setup);
    setup.getPlanet()->setFlags(game::sim::Planet::fl_DoubleBeamChargeSet);

    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    game::sim::Loader testee(cs);

    afl::io::InternalStream stream;
    testee.save(stream, setup);

    static const uint8_t EXPECTED[] = {
        // Header (10)
        'C','C','b','s','i','m','4',26,1,0x80,

        // Ship (67)
        'B','o','a','t',32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,    // 20
        0,0, 17,0, 1,0, 3,0, 0,0, 0,0, 0,0, 0,0, 7,0, 180,0, 4,0,           // 22
        8,0, 0,0, 100,0, 'a','b','c', 0,0, 100,0, 0,0,
        0,0,0,0, 0,0, 0,0, 0,0,

        // Planet (67)
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,                   // 20
        0,0, 0,0, 1,0, 4,0, 0,0, 6,0, 0,0, 0,0, 0,0, 0,0, 9,0,              // 22
        12,0, 61,0, 100,0, 'x','y','z', 0xFF,0xFF, 0,0, 0,0,
        0,0,0,0, 0,0, 0,0, 8,0,
    };

    TS_ASSERT_EQUALS(stream.getContent().size(), sizeof(EXPECTED));
    TS_ASSERT_SAME_DATA(stream.getContent().at(0), EXPECTED, sizeof(EXPECTED));
}

