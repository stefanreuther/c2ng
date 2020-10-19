/**
  *  \file u/t_game_vcr_classic_hostalgorithm.cpp
  *  \brief Test for game::vcr::classic::HostAlgorithm
  *
  *  Test cases ported from JavaScript version (js/projects/c2web/game/tvcr.js 1.13).
  */

#include <memory>
#include "game/vcr/classic/hostalgorithm.hpp"

#include "t_game_vcr_classic.hpp"
#include "afl/base/countof.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"
#include "game/vcr/classic/statustoken.hpp"
#include "game/test/shiplist.hpp"


namespace {
    void initShipList(game::spec::ShipList& list)
    {
        game::test::initStandardBeams(list);
        game::test::initStandardTorpedoes(list);
    }

    /*
     *  Hardwired combat to avoid dependency on external files.
     *  These fights are from actual games.
     */
    struct Object {
        int mass;
        int isPlanet;
        const char* name;
        int damage;
        int crew;
        int id;
        int owner;
        int image;
        int hull;
        int beamType;
        int numBeams;
        int experienceLevel;
        int numBays;
        int torpedoType;
        int numTorpedoes;
        int numFighters;
        int numLaunchers;
        int shield;
        int beamKillRate;
        int beamChargeRate;
        int torpMissRate;
        int torpChargeRate;
        int crewDefenseRate;
    };
    struct Battle {
        uint16_t seed;
        int magic;
        int capabilities;
        Object object[2];
    };

    const Battle battles[] = {
        // This is pcc-v2/tests/vcr/vcr2.dat:
        //             mass pl name                     da crw   id pl  im hu bt nb xp bay tt nt  nf  nl  sh  nuConfig
        {42,  0, 0,  {{150, 0, "KotSCHa PoX",           0,   2,  14, 2, 31, 0, 0, 0, 0, 0,  0, 0,  0, 0, 100, 1,1,35,1,0},
                      {233, 0, "SDR Dauthi Shadow",     0, 240, 434, 3, 61, 0, 5, 6, 0, 0,  7, 0,  0, 4, 100, 1,1,35,1,0}}},
        {99,  0, 0,  {{280, 0, "LSS KoloSS doX  pHA",   0, 430, 365, 2, 46, 0, 7, 4, 0, 0,  7, 20, 0, 3, 100, 1,1,35,1,0},
                      {233, 0, "STR The Dauthi >>#00",  0, 240, 447, 3, 61, 0, 4, 6, 0, 0, 10, 35, 0, 4, 100, 1,1,35,1,0}}},
        {30,  0, 0,  {{158, 0, "Roxen SCHaloSS dUl",    0, 102,  70, 2, 76, 0, 4, 4, 0, 0,  0, 0,  0, 0, 100, 1,1,35,1,0},
                      {233, 0, "STR Dauthi Slayer",     0, 240, 470, 3, 61, 0, 4, 6, 0, 0, 10, 35, 0, 4, 100, 1,1,35,1,0}}},
        {35,  0, 0,  {{45,  0, "Jokabon Solaris 45",    0,  78,  71, 2, 29, 0, 2, 2, 0, 0,  0, 0,  0, 0, 100, 1,1,35,1,0},
                      {233, 0, "STR Dauthi Slayer",     0, 240, 470, 3, 61, 0, 4, 6, 0, 0, 10, 31, 0, 4, 100, 1,1,35,1,0}}},
        {64,  0, 0,  {{45,  0, "Golem DaXschok Ales",   0,  78,  77, 2, 29, 0, 1, 2, 0, 0,  0, 0,  0, 0, 100, 1,1,35,1,0},
                      {233, 0, "STR Dauthi Slayer",     0, 240, 470, 3, 61, 0, 4, 6, 0, 0, 10, 30, 0, 4, 100, 1,1,35,1,0}}},
        {72,  0, 0,  {{198, 0, "SoXa domaSCH KoX",      0, 102, 489, 2, 19, 0, 0, 0, 0, 0,  0, 0,  0, 0, 100, 1,1,35,1,0},
                      {233, 0, "STR Dauthi Slayer",     0, 240, 470, 3, 61, 0, 4, 6, 0, 0, 10, 28, 0, 4, 100, 1,1,35,1,0}}},
        {103, 0, 0,  {{55,  0, "Hissen iss schoen! 04", 0,  35, 111, 2, 49, 0, 2, 2, 0, 0,  0, 0,  0, 0, 100, 1,1,35,1,0},
                      {233, 0, "HKF Panther Eness",     0, 240,  58, 3, 61, 0, 5, 6, 0, 0, 10, 35, 0, 4, 100, 1,1,35,1,0}}},
        {88,  0, 0,  {{55,  0, "Hissen iss schoen! 05", 0,  35, 454, 2, 49, 0, 2, 2, 0, 0,  0, 0,  0, 0, 100, 1,1,35,1,0},
                      {233, 0, "HKF Panther Eness",     0, 240,  58, 3, 61, 0, 5, 6, 0, 0, 10, 31, 0, 4, 100, 1,1,35,1,0}}},
        {109, 0, 0,  {{45,  0, "JaloXa Duschan 264-5",  0,  78,  33, 2, 29, 0, 2, 2, 0, 0,  0, 0,  0, 0, 100, 1,1,35,1,0},
                      {181, 0, "HKF Shiman Eness",      0, 240, 114, 3, 61, 0, 5, 6, 0, 0,  7, 35, 0, 4, 100, 1,1,35,1,0}}},
        {55,  0, 0,  {{228, 0, "LCC 1729 Gobi SuXol",   0, 430, 237, 2, 46, 0, 2, 4, 0, 0,  4, 30, 0, 3, 100, 1,1,35,1,0},
                      {181, 0, "HKF Shiman Eness",      0, 240, 114, 3, 61, 0, 5, 6, 0, 0,  7, 32, 0, 4, 100, 1,1,35,1,0}}},
        {56,  0, 0,  {{128, 0, "Kohlem DaXTscho 83-d",  0,   6,  23, 2, 32, 0, 0, 0, 0, 0,  0, 0,  0, 0, 100, 1,1,35,1,0},
                     {181,  0, "HKF Spirit of Eness",   0, 240, 115, 3, 61, 0, 5, 6, 0, 0,  7, 30, 0, 4, 100, 1,1,35,1,0}}},
        {73,  0, 50, {{113, 0, "SDR Dauthi Shadow",     0, 240, 434, 3, 61, 0, 5, 6, 0, 0,  7, 0,  0, 4, 100, 1,1,35,1,0},
                      {227, 1, "Crete",                 0,  31, 106, 2,  1, 0, 8, 7, 0, 16, 0, 0, 31, 0, 100, 1,1,35,1,0}}},
        {105, 0, 48, {{113, 0, "STR Dauthi Slayer",     0, 240, 470, 3, 61, 0, 4, 6, 0, 0, 10, 24, 0, 4, 100, 1,1,35,1,0},
                      {157, 1, "Tniacth",               0,   8, 483, 2,  1, 0, 5, 4, 0, 8,  0, 0,  8, 0, 100, 1,1,35,1,0}}},
        {52,  0, 50, {{113, 0, "DSC Nether Shadow >#",  0, 240, 374, 3, 61, 0, 5, 6, 0, 0,  7, 20, 0, 4, 100, 1,1,35,1,0},
                      {227, 1, "Crete",                 0,  14, 106, 2,  1, 0, 8, 7, 0, 16, 0, 0, 14, 0, 100, 1,1,35,1,0}}},
        {6,   0, 77, {{113, 0, "HKF Panther Eness",     0, 240,  58, 3, 61, 0, 5, 6, 0, 0, 10, 28, 0, 4, 100, 1,1,35,1,0},
                      {144, 1, "Daventhor",             0,   6, 453, 2,  1, 0, 4, 4, 0, 6,  0, 0,  6, 0, 100, 1,1,35,1,0}}},
        {46,  0, 34, {{113, 0, "HKF Shiman Eness",      0, 240, 114, 3, 61, 0, 5, 6, 0, 0,  7, 10, 0, 4,  65, 1,1,35,1,0},
                      {125, 1, "Organia",               0,   5,  53, 2,  1, 0, 4, 3, 0, 5,  0, 0,  5, 0, 100, 1,1,35,1,0}}},
        {65,  0, 72, {{113, 0, "HKF Spirit of Eness",   0, 240, 115, 3, 61, 0, 5, 6, 0, 0,  7, 27, 0, 4, 100, 1,1,35,1,0},
                      {123, 1, "Cygnet",                0,   5,  41, 2,  1, 0, 3, 3, 0, 5,  0, 0,  5, 0, 100, 1,1,35,1,0}}},

        // This is pcc-v2/tests/vcr/deadfire.vcr, a carrier/carrier fight:
        {107, 0, 47, {{625, 0, "Carota", 0, 1858, 496, 11, 144, 0, 7, 10, 0, 8, 0, 0, 122, 0, 100, 1,1,35,1,0},
                      {370, 1, "Vendor", 0, 62,   32,  1,  1,   0, 6, 9, 0, 13, 0, 0, 62,  0, 100, 1,1,35,1,0}}}
    };

    game::vcr::Object convertObject(const Object& in)
    {
        game::vcr::Object result;
        result.setMass(in.mass);
        result.setIsPlanet(in.isPlanet);
        result.setName(in.name);
        result.setDamage(in.damage);
        result.setCrew(in.crew);
        result.setId(in.id);
        result.setOwner(in.owner);
        result.setPicture(in.image);
        result.setHull(in.hull);
        result.setBeamType(in.beamType);
        result.setNumBeams(in.numBeams);
        result.setExperienceLevel(in.experienceLevel);
        result.setNumBays(in.numBays);
        result.setTorpedoType(in.torpedoType);
        result.setNumTorpedoes(in.numTorpedoes);
        result.setNumFighters(in.numFighters);
        result.setNumLaunchers(in.numLaunchers);
        result.setShield(in.shield);
        result.setBeamKillRate(in.beamKillRate);
        result.setBeamChargeRate(in.beamChargeRate);
        result.setTorpMissRate(in.torpMissRate);
        result.setTorpChargeRate(in.torpChargeRate);
        result.setCrewDefenseRate(in.crewDefenseRate);

        // The objects are derived from real VCR files. Since we moved applyClassicLimits() from VCR core
        // to the loader, we need to do it here as well.
        result.applyClassicLimits();

        return result;
    }
}

/** Test first battle: Freighter vs Torper, normal playback.
    Must produce correct result. */
void
TestGameVcrClassicHostAlgorithm::testFirst()
{
    // Surroundings
    game::vcr::classic::NullVisualizer vis;
    game::config::HostConfiguration config;
    game::spec::ShipList list;
    initShipList(list);

    // First fight
    game::vcr::classic::HostAlgorithm testee(false, vis, config, list.beams(), list.launchers());
    game::vcr::Object left(convertObject(battles[0].object[0]));
    game::vcr::Object right(convertObject(battles[0].object[1]));
    uint16_t seed = battles[0].seed;
    bool result = testee.checkBattle(left, right, seed);
    TS_ASSERT(!result);

    testee.initBattle(left, right, seed);
    while (testee.playCycle()) {
        // nix
    }
    testee.doneBattle(left, right);

    // Record #0:
    // 	Ending time 193 (3:13)
    // 	left-captured
    //   S:  0  D:  9  C:  0  A:  0   |     S:100  D:  0  C:240  A:  0
    TS_ASSERT_EQUALS(testee.getTime(), 193);
    TS_ASSERT(testee.getResult().contains(game::vcr::classic::LeftCaptured));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightCaptured));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightDestroyed));
    TS_ASSERT_EQUALS(left.getShield(), 0);
    TS_ASSERT_EQUALS(right.getShield(), 100);
    TS_ASSERT_EQUALS(left.getDamage(), 9);
    TS_ASSERT_EQUALS(right.getDamage(), 0);
    TS_ASSERT_EQUALS(left.getCrew(), 0);
    TS_ASSERT_EQUALS(right.getCrew(), 240);
    TS_ASSERT_EQUALS(testee.getStatistic(game::vcr::classic::LeftSide).getNumFights(), 1);
    TS_ASSERT_EQUALS(testee.getStatistic(game::vcr::classic::RightSide).getNumFights(), 1);
}

/** Test second battle: Torper vs Torper, normal playback.
    Must produce correct result. */
void
TestGameVcrClassicHostAlgorithm::testSecond()
{
    // Surroundings
    game::vcr::classic::NullVisualizer vis;
    game::config::HostConfiguration config;
    game::spec::ShipList list;
    initShipList(list);

    // Second fight
    game::vcr::classic::HostAlgorithm testee(false, vis, config, list.beams(), list.launchers());
    game::vcr::Object left(convertObject(battles[1].object[0]));
    game::vcr::Object right(convertObject(battles[1].object[1]));
    uint16_t seed = battles[1].seed;
    bool result = testee.checkBattle(left, right, seed);
    TS_ASSERT(!result);

    testee.initBattle(left, right, seed);
    while (testee.playCycle()) {
        // nix
    }
    testee.doneBattle(left, right);

    // Record #2:
    //         Ending time 291 (4:51)
    //         right-destroyed
    //   S:  0  D:143  C:169  A:  5   |     S:  0  D:102  C:121  A: 15
    TS_ASSERT_EQUALS(testee.getTime(), 291);
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftCaptured));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightCaptured));
    TS_ASSERT(testee.getResult().contains(game::vcr::classic::RightDestroyed));
    TS_ASSERT_EQUALS(left.getShield(), 0);
    TS_ASSERT_EQUALS(right.getShield(), 0);
    TS_ASSERT_EQUALS(left.getDamage(), 143);
    TS_ASSERT_EQUALS(right.getDamage(), 102);
    TS_ASSERT_EQUALS(left.getCrew(), 169);
    TS_ASSERT_EQUALS(right.getCrew(), 121);
}

/** Test last battle: Torper vs Planet, normal playback.
    Must produce correct result. */
void
TestGameVcrClassicHostAlgorithm::testLast()
{
    // Surroundings
    game::vcr::classic::NullVisualizer vis;
    game::config::HostConfiguration config;
    game::spec::ShipList list;
    initShipList(list);

    // Final recording (ship/planet)
    game::vcr::classic::HostAlgorithm testee(false, vis, config, list.beams(), list.launchers());
    game::vcr::Object left(convertObject(battles[16].object[0]));
    game::vcr::Object right(convertObject(battles[16].object[1]));
    uint16_t seed = battles[16].seed;
    bool result = testee.checkBattle(left, right, seed);
    TS_ASSERT(!result);

    testee.initBattle(left, right, seed);
    while (testee.playCycle()) {
        // nix
    }
    testee.doneBattle(left, right);

    // Record #17:
    //         Ending time 344 (5:44)
    //         right-destroyed
    //   S: 58  D:  0  C:240  A: 11   |     S:  0  D:220  C:  5  A:  0
    TS_ASSERT_EQUALS(testee.getTime(), 344);
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftCaptured));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightCaptured));
    TS_ASSERT(testee.getResult().contains(game::vcr::classic::RightDestroyed));
    TS_ASSERT_EQUALS(left.getShield(), 58);
    TS_ASSERT_EQUALS(right.getShield(), 0);
    TS_ASSERT_EQUALS(left.getDamage(), 0);
    TS_ASSERT_EQUALS(right.getDamage(), 220);
    TS_ASSERT_EQUALS(left.getCrew(), 240);
    TS_ASSERT_EQUALS(right.getCrew(), 5);
}

/** Test fighter/fighter, normal playback.
    Must produce correct result. */
void
TestGameVcrClassicHostAlgorithm::testDeadFire()
{
    // Surroundings
    game::vcr::classic::NullVisualizer vis;
    game::config::HostConfiguration config;
    game::spec::ShipList list;
    initShipList(list);

    // "Deadfire" fight (carrier/carrier fight)
    game::vcr::classic::HostAlgorithm testee(false, vis, config, list.beams(), list.launchers());
    game::vcr::Object left(convertObject(battles[17].object[0]));
    game::vcr::Object right(convertObject(battles[17].object[1]));
    uint16_t seed = battles[17].seed;
    bool result = testee.checkBattle(left, right, seed);
    TS_ASSERT(!result);

    testee.initBattle(left, right, seed);
    while (testee.playCycle()) {
        // nix
    }
    testee.doneBattle(left, right);

    // Record #18:
    //         Ending time 363 (6:03)
    //         right-destroyed
    //  S:  5  D:  0  C:1858  A: 65   |     S:  0  D:102  C: 62  A:  0
    TS_ASSERT_EQUALS(testee.getTime(), 363);
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftCaptured));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightCaptured));
    TS_ASSERT(testee.getResult().contains(game::vcr::classic::RightDestroyed));
    TS_ASSERT_EQUALS(left.getShield(), 5);
    TS_ASSERT_EQUALS(right.getShield(), 0);
    TS_ASSERT_EQUALS(left.getDamage(), 0);
    TS_ASSERT_EQUALS(right.getDamage(), 102);
    TS_ASSERT_EQUALS(left.getCrew(), 1858);
}

/** Test tenth battle: Torper vs Torper.
    This also tests partial playback, intermediate status queries, and status tokens.
    Must produce correct result at all stages. */
void
TestGameVcrClassicHostAlgorithm::testTenth()
{
    using game::vcr::classic::LeftSide;
    using game::vcr::classic::RightSide;

    // Surroundings
    game::vcr::classic::NullVisualizer vis;
    game::config::HostConfiguration config;
    game::spec::ShipList list;
    initShipList(list);

    // Final recording (ship/planet)
    game::vcr::classic::HostAlgorithm testee(false, vis, config, list.beams(), list.launchers());
    game::vcr::Object left(convertObject(battles[9].object[0]));
    game::vcr::Object right(convertObject(battles[9].object[1]));
    uint16_t seed = battles[9].seed;
    bool result = testee.checkBattle(left, right, seed);
    TS_ASSERT(!result);

    // Run until time 150 (2:30)
    testee.initBattle(left, right, seed);
    for (int i = 0; i < 150; ++i) {
        TS_ASSERT_EQUALS(testee.playCycle(), true);
    }

    // Verify intermediate state
    TS_ASSERT_EQUALS(testee.getTime(), 150);
    TS_ASSERT_EQUALS(testee.getShield(LeftSide), 50);
    TS_ASSERT_EQUALS(testee.getShield(RightSide), 94);
    TS_ASSERT_EQUALS(testee.getDamage(LeftSide), 0);
    TS_ASSERT_EQUALS(testee.getDamage(RightSide), 0);
    TS_ASSERT_EQUALS(testee.getCrew(LeftSide), 430);
    TS_ASSERT_EQUALS(testee.getCrew(RightSide), 240);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(LeftSide), 28);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(RightSide), 28);
    for (int i = 0; i < 4; ++i) {
        TS_ASSERT_EQUALS(testee.getBeamStatus(LeftSide, i), 100);
    }
    for (int i = 0; i < 6; ++i) {
        TS_ASSERT_EQUALS(testee.getBeamStatus(RightSide, i), 100);
    }
    TS_ASSERT_EQUALS(testee.getLauncherStatus(LeftSide, 0), 17);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(LeftSide, 1), 100);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(LeftSide, 2), 20);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(RightSide, 0), 12);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(RightSide, 1), 22);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(RightSide, 2), 17);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(RightSide, 3), 17);

    // Save a token
    std::auto_ptr<game::vcr::classic::StatusToken> token(testee.createStatusToken());
    TS_ASSERT(token.get() != 0);

    // Run until time 210 (3:30)
    for (int i = 0; i < 60; ++i) {
        TS_ASSERT_EQUALS(testee.playCycle(), true);
    }

    // Verify intermediate state
    TS_ASSERT_EQUALS(testee.getTime(), 210);
    TS_ASSERT_EQUALS(testee.getShield(LeftSide), 0);
    TS_ASSERT_EQUALS(testee.getShield(RightSide), 81);
    TS_ASSERT_EQUALS(testee.getDamage(LeftSide), 60);
    TS_ASSERT_EQUALS(testee.getDamage(RightSide), 0);
    TS_ASSERT_EQUALS(testee.getCrew(LeftSide), 376);
    TS_ASSERT_EQUALS(testee.getCrew(RightSide), 240);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(LeftSide), 23);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(RightSide), 21);
    TS_ASSERT_EQUALS(testee.getBeamStatus(LeftSide, 0), 6);
    TS_ASSERT_EQUALS(testee.getBeamStatus(LeftSide, 1), 13);
    TS_ASSERT_EQUALS(testee.getBeamStatus(LeftSide, 2), 8);
    TS_ASSERT_EQUALS(testee.getBeamStatus(LeftSide, 3), 12);
    TS_ASSERT_EQUALS(testee.getBeamStatus(RightSide, 0), 9);
    TS_ASSERT_EQUALS(testee.getBeamStatus(RightSide, 1), 4);
    TS_ASSERT_EQUALS(testee.getBeamStatus(RightSide, 2), 9);
    TS_ASSERT_EQUALS(testee.getBeamStatus(RightSide, 3), 10);
    TS_ASSERT_EQUALS(testee.getBeamStatus(RightSide, 4), 7);
    TS_ASSERT_EQUALS(testee.getBeamStatus(RightSide, 4), 7);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(LeftSide, 0), 2);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(LeftSide, 1), 45);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(LeftSide, 2), 92);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(RightSide, 0), 80);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(RightSide, 1), 12);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(RightSide, 2), 7);
    TS_ASSERT_EQUALS(testee.getLauncherStatus(RightSide, 3), 7);

    // Restore the token
    testee.restoreStatus(*token);
    TS_ASSERT_EQUALS(testee.getTime(), 150);
    TS_ASSERT_EQUALS(testee.getShield(LeftSide), 50);
    TS_ASSERT_EQUALS(testee.getShield(RightSide), 94);

    // Play again
    for (int i = 0; i < 60; ++i) {
        TS_ASSERT_EQUALS(testee.playCycle(), true);
    }
    TS_ASSERT_EQUALS(testee.getTime(), 210);
    TS_ASSERT_EQUALS(testee.getShield(LeftSide), 0);
    TS_ASSERT_EQUALS(testee.getShield(RightSide), 81);

    // Play to end
    while (testee.playCycle()) {
        // nix
    }
    testee.doneBattle(left, right);

    // Record #10:
    //        Ending time 302 (5:02)
    //        left-destroyed
    //  S:  0  D:158  C:268  A: 16   |     S: 65  D:  0  C:240  A: 10
    TS_ASSERT_EQUALS(testee.getTime(), 302);
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::LeftCaptured));
    TS_ASSERT(testee.getResult().contains(game::vcr::classic::LeftDestroyed));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightCaptured));
    TS_ASSERT(!testee.getResult().contains(game::vcr::classic::RightDestroyed));
    TS_ASSERT_EQUALS(left.getShield(), 0);
    TS_ASSERT_EQUALS(right.getShield(), 65);
    TS_ASSERT_EQUALS(left.getDamage(), 158);
    TS_ASSERT_EQUALS(right.getDamage(), 0);
    TS_ASSERT_EQUALS(left.getCrew(), 268);
    TS_ASSERT_EQUALS(right.getCrew(), 240);
    TS_ASSERT_EQUALS(left.getNumTorpedoes(), 16);
    TS_ASSERT_EQUALS(right.getNumTorpedoes(), 10);
}

