/**
  *  \file u/t_game_sim_loader.cpp
  *  \brief Test for game::sim::Loader
  *
  *  These test cases use actual .ccb files created by PCC/CCBSim/PCC2/PlayVCR.
  */

#include "game/sim/loader.hpp"

#include "t_game_sim.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/io/constmemorystream.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/planet.hpp"
#include "afl/except/fileformatexception.hpp"
#include "u/files.hpp"

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
