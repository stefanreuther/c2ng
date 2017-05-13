/**
  *  \file u/t_game_vcr_classic_hostalgorithm.cpp
  *  \brief Test for game::vcr::classic::HostAlgorithm
  *
  *  Test cases ported from JavaScript version (js/projects/c2web/game/tvcr.js 1.13).
  */

#include "game/vcr/classic/hostalgorithm.hpp"

#include "t_game_vcr_classic.hpp"
#include "afl/base/countof.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"


namespace {
    struct Cost {
        int d, m, mc, t;
    };
    struct Beam {
        Cost cost;
        int damagePower;
        int killPower;
        int mass;
        const char* name;
        int techLevel;
    };
    const Beam beams[] = {
        {{0,0,1,1},3,10,1,"Laser",1},
        {{0,0,2,1},1,15,1,"X-Ray Laser",1},
        {{2,0,5,1},10,3,2,"Plasma Bolt",2},
        {{12,1,10,1},25,10,4,"Blaster",3},
        {{12,5,12,1},29,9,3,"Positron Beam",4},
        {{12,1,13,1},20,30,4,"Disruptor",5},
        {{12,14,31,1},40,20,7,"Heavy Blaster",6},
        {{12,30,35,1},35,30,5,"Phaser",7},
        {{17,37,36,1},35,50,7,"Heavy Disruptor",8},
        {{12,55,54,1},45,35,6,"Heavy Phaser",10}
    };
    struct Torpedo {
        const char* name;
        Cost torpedoCost;
        Cost launcherCost;
        int mass;
        int techLevel;
        int killPower;
        int damagePower;
    };
    const Torpedo torpedoes[] = {
        {"Mark 1 Photon", {1,1,1,1},  {1,1,1,0},  2,1,4,5},
        {"Proton torp",   {2,1,1,1},  {4,1,0,0},  2,2,6,8},
        {"Mark 2 Photon", {5,1,1,1},  {4,1,4,0},  2,3,3,10},
        {"Gamma Bomb",    {10,1,1,1}, {6,1, 3,1}, 4,3,15,2},
        {"Mark 3 Photon", {12,1,1,1}, {5,1,1,5},  2,4,9,15},
        {"Mark 4 Photon", {13,1,1,1}, {20,1,4,1}, 2,5,13,30},
        {"Mark 5 Photon", {31,1,1,1}, {57,1,7,14},3,6,17,35},
        {"Mark 6 Photon", {35,1,1,1}, {100,1,2,7},2,7,23,40},
        {"Mark 7 Photon", {36,1,1,1}, {120,1,3,8},3,8,25,48},
        {"Mark 8 Photon", {54,1,1,1}, {190,1,1,9},3,10,35,55}
    };

    game::spec::Cost convertCost(const Cost& c)
    {
        game::spec::Cost result;
        result.set(result.Duranium, c.d);
        result.set(result.Tritanium, c.t);
        result.set(result.Molybdenum, c.m);
        result.set(result.Money, c.mc);
        return result;
    }

    void initShipList(game::spec::ShipList& list)
    {
        for (int i = 0; i < int(countof(beams)); ++i) {
            const Beam& in = beams[i];
            if (game::spec::Beam* out = list.beams().create(i+1)) {
                out->setKillPower(in.killPower);
                out->setDamagePower(in.damagePower);
                out->setMass(in.mass);
                out->setTechLevel(in.techLevel);
                out->setName(in.name);
                out->cost() = convertCost(in.cost);
            }
        }
        for (int i = 0; i < int(countof(torpedoes)); ++i) {
            const Torpedo& in = torpedoes[i];
            if (game::spec::TorpedoLauncher* out = list.launchers().create(i+1)) {
                out->setKillPower(in.killPower);
                out->setDamagePower(in.damagePower);
                out->setMass(in.mass);
                out->setTechLevel(in.techLevel);
                out->setName(in.name);
                out->cost() = convertCost(in.launcherCost);
                out->torpedoCost() = convertCost(in.torpedoCost);
            }
        }
    }




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

        // tests/vcr/deadfire.vcr, a carrier/carrier fight:
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
        return result;
    }

}


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
}

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
