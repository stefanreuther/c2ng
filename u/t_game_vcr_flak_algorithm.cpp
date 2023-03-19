/**
  *  \file u/t_game_vcr_flak_algorithm.cpp
  *  \brief Test for game::vcr::flak::Algorithm
  */

#include "game/vcr/flak/algorithm.hpp"

#include "t_game_vcr_flak.hpp"
#include "afl/base/countof.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/componentvector.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/flak/configuration.hpp"
#include "game/vcr/flak/gameenvironment.hpp"
#include "game/vcr/flak/nullvisualizer.hpp"
#include "game/vcr/flak/setup.hpp"

namespace {
    uint8_t FILE_CONTENT[] = {
        0xb8, 0x02, 0x00, 0x00, 0x23, 0x0a, 0xde, 0x09, 0xc9, 0x7a, 0x3d, 0x6d, 0x60, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x88, 0x02, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x64, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xe0, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x65, 0x42, 0x00, 0x00, 0x29, 0x01, 0x00, 0x00, 0x09, 0x00, 0x04, 0x00, 0x02, 0x00, 0x64, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xe3, 0x55, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x04, 0x00, 0x06, 0x00, 0x02, 0x00, 0x64, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xa0, 0x92, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x52, 0x4b, 0x20, 0x42, 0x61, 0x72, 0x69, 0x75,
        0x6d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x6e, 0x00,
        0x2b, 0x00, 0x09, 0x00, 0x51, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x0c, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x64, 0x00, 0x01, 0x00, 0x83, 0x00, 0x00, 0x00,
        0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x4b, 0x20, 0x47, 0x69, 0x62, 0x61, 0x72, 0x69, 0x61,
        0x6e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x0b, 0x04, 0xc9, 0x00,
        0x09, 0x00, 0x53, 0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x36, 0x00, 0x6f, 0x01, 0x64, 0x00, 0x10, 0x00, 0xf5, 0x01, 0x00, 0x00, 0xf4, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x52, 0x4b, 0x20, 0x4e, 0x69, 0x74, 0x72, 0x6f, 0x67, 0x65, 0x6e, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x0b, 0x04, 0x36, 0x01, 0x09, 0x00,
        0x53, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x36, 0x00, 0xe2, 0x01, 0x64, 0x00, 0x10, 0x00, 0x7c, 0x02, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00,
        0xff, 0xff, 0x54, 0x68, 0x65, 0x74, 0x61, 0x20, 0x56, 0x49, 0x49, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0xba, 0x01, 0x09, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x07, 0x00, 0x0a, 0x00, 0x06, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0d, 0x00, 0x26, 0x00,
        0xe6, 0x00, 0x64, 0x00, 0x1a, 0x00, 0xca, 0x01, 0x00, 0x00, 0xf4, 0x01, 0x01, 0x00, 0x00, 0x00,
        0x52, 0x4b, 0x20, 0x56, 0x61, 0x6e, 0x64, 0x69, 0x75, 0x6d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x06, 0x08, 0xb4, 0x02, 0x09, 0x00, 0x4f, 0x00, 0x01, 0x00,
        0x07, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x69, 0x00, 0x21, 0x03,
        0x64, 0x00, 0x20, 0x00, 0xe7, 0x03, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x52, 0x4b,
        0x20, 0x53, 0x74, 0x72, 0x6f, 0x6e, 0x74, 0x69, 0x75, 0x6d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x00, 0x00, 0x06, 0x08, 0xce, 0x03, 0x09, 0x00, 0x4f, 0x00, 0x01, 0x00, 0x07, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x5f, 0x00, 0x53, 0x03, 0x64, 0x00,
        0x20, 0x00, 0x19, 0x04, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x75, 0x72, 0x74,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x00, 0x00, 0x13, 0x03, 0x96, 0x01, 0x04, 0x00, 0x23, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x09, 0x00,
        0x0d, 0x00, 0x59, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x02, 0x64, 0x00, 0x01, 0x00,
        0xa2, 0x03, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0xff, 0xff, 0x47, 0x72, 0x61, 0x75, 0x74, 0x76,
        0x6f, 0x72, 0x6e, 0x69, 0x78, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
        0x13, 0x03, 0xd1, 0x02, 0x04, 0x00, 0x23, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x09, 0x00, 0x0d, 0x00,
        0x64, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x02, 0x64, 0x00, 0x01, 0x00, 0xa2, 0x03,
        0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0xff, 0xff, 0x06, 0x00, 0x32, 0x00, 0x07, 0x00, 0x2a, 0x00,
        0x06, 0x00, 0x1f, 0x00, 0x07, 0x00, 0x1c, 0x00, 0x06, 0x00, 0x2f, 0x00, 0x07, 0x00, 0x28, 0x00,
        0x00, 0x00, 0x33, 0x00, 0x01, 0x00, 0x32, 0x00, 0x02, 0x00, 0x3a, 0x00, 0x03, 0x00, 0x2a, 0x00,
        0x04, 0x00, 0x2a, 0x00, 0x05, 0x00, 0x2c, 0x00
    };

    const uint8_t ONE_ON_ONE_CONTENT[] = {
        0xec, 0x00, 0x00, 0x00, 0xe8, 0x03, 0xe8, 0x03, 0x95, 0xec, 0x60, 0x92, 0xf1, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0xe4, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x4b, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xa0, 0x92, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x01, 0x00, 0x01, 0x00, 0x4b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x60, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x66, 0x00,
        0x64, 0x00, 0x05, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x04, 0x00, 0x09, 0x00, 0x04, 0x00, 0x32, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0x00, 0x64, 0x00, 0x01, 0x00, 0xa2, 0x00, 0x00, 0x00,
        0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x06, 0x00, 0xc8, 0x00,
        0x06, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x05, 0x00, 0x01, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x22, 0x00
    };


    /* Environment. For simplicity, we use a GameEnvironment and build its environment,
       instead of making a full Environment mock. */
    struct TestEnvironment {
        game::config::HostConfiguration config;
        game::spec::BeamVector_t beams;
        game::spec::TorpedoVector_t torps;
        game::vcr::flak::GameEnvironment env;

        TestEnvironment()
            : config(), beams(), torps(), env(config, beams, torps)
            { }
    };

    void initConfig(TestEnvironment& env)
    {
        // Host configuration from game "FLAK0"
        static const char*const OPTIONS[][2] = {
            { "EModBayRechargeRate",      "4,8,5,0" },
            { "EModBayRechargeBonus",     "0,0,0,0" },
            { "EModBeamRechargeRate",     "0,1,1,2" },
            { "EModBeamRechargeBonus",    "0,0,1,1" },
            { "EModTubeRechargeRate",     "1,2,3,5" },
            { "EModBeamHitFighterCharge", "0,0,0,0" },
            { "EModTorpHitOdds",          "1,2,3,5" },
            { "EModBeamHitOdds",          "4,4,5,8" },
            { "EModBeamHitBonus",         "2,2,3,5" },
            { "EModStrikesPerFighter",    "0,0,0,1" },
            { "EModFighterBeamExplosive", "0,0,0,0" },
            { "EModFighterBeamKill",      "0,0,0,0" },
            { "EModFighterMovementSpeed", "0,0,0,0" },
            { "EModTorpHitBonus",         "1,2,3,4" },
            { "EModTubeRechargeBonus",    "1,1,2,3" },
            { "EModShieldDamageScaling",  "0" },
            { "EModShieldKillScaling",    "0" },
            { "EModHullDamageScaling",    "0" },
            { "EModCrewKillScaling",      "-3,-6,-9,-12" },
            { "AllowAlternativeCombat",   "Yes" },
            { "BeamFiringRange",          "25000" },
            { "BeamHitShipCharge",        "600" },
            { "BeamHitFighterCharge",     "460" },
            { "BeamHitOdds",              "70" },
            { "BeamHitBonus",             "12" },
            { "BeamRechargeRate",         "4" },
            { "BeamRechargeBonus",        "4" },
            { "FireOnAttackFighters",     "Yes" },
            { "BayLaunchInterval",        "2" },
            { "BayRechargeRate",          "40" },
            { "BayRechargeBonus",         "1" },
            { "FighterBeamExplosive",     "9" },
            { "FighterBeamKill",          "9" },
            { "FighterFiringRange",       "3000" },
            { "FighterKillOdds",          "0" },
            { "FighterMovementSpeed",     "300" },
            { "PlayerRace",               "1,2,3,4,5,6,7,8,9,10,11" },
            { "StrikesPerFighter",        "5" },
            { "TorpFiringRange",          "30000" },
            { "TorpHitOdds",              "50" },
            { "TorpHitBonus",             "13" },
            { "TubeRechargeRate",         "30" },
            { "TubeRechargeBonus",        "7" },
            { "CrewKillScaling",          "30" },
            { "HullDamageScaling",        "20" },
            { "ShieldDamageScaling",      "40" },
            { "ShieldKillScaling",        "0" },
            { "ShipMovementSpeed",        "100" },
            { "StandoffDistance",         "10000" },
        };
        for (size_t i = 0; i < countof(OPTIONS); ++i) {
            env.config.setOption(OPTIONS[i][0], OPTIONS[i][1], game::config::ConfigurationOption::Game);
        }
    }

    void initBeams(TestEnvironment& env)
    {
        // Beams from game FLAK0
        //                            Las KOZ Dis Pha Dis ERa Ion TlB Inp MtS
        static const int KILL[]   = {  1, 10,  7, 15, 40, 20, 10, 45, 70, 40 };
        static const int DAMAGE[] = {  3,  1, 10, 25, 10, 40, 60, 55, 35, 80 };
        for (int i = 1; i <= 10; ++i) {
            game::spec::Beam* b = env.beams.create(i);
            b->setKillPower(KILL[i-1]);
            b->setDamagePower(DAMAGE[i-1]);
        }
    }

    void initTorpedoes(TestEnvironment& env)
    {
        // Torpedoes from game FLAK0
        //                            SpR PMB FuB InB PhT Gra Ark AmB Kat SFD
        static const int KILL[]   = { 10, 60, 25, 60, 15, 30, 60, 25, 80, 50 };
        static const int DAMAGE[] = { 25,  3, 50, 20, 82, 75, 50, 90, 40, 99 };
        for (int i = 1; i <= 10; ++i) {
            game::spec::TorpedoLauncher* tl = env.torps.create(i);
            tl->setKillPower(KILL[i-1]);
            tl->setDamagePower(DAMAGE[i-1]);
        }
    }

    void init(TestEnvironment& env)
    {
        initConfig(env);
        initBeams(env);
        initTorpedoes(env);
    }
}

/** Test playback.
    A: load a buffer. Play it.
    E: check against results from PCC2 implementation. */
void
TestGameVcrFlakAlgorithm::testPlay()
{
    // Environment
    TestEnvironment env;
    afl::string::NullTranslator tx;
    init(env);

    // Test
    game::vcr::flak::Setup testee;
    afl::charset::Utf8Charset cs;
    testee.load("testPlay", FILE_CONTENT, cs, tx);

    game::vcr::flak::NullVisualizer vis;
    game::vcr::flak::Algorithm algo(testee, env.env);
    algo.init(env.env, vis);

    // Play to time 100
    while (algo.getTime() < 100) {
        TS_ASSERT(algo.playCycle(env.env, vis));
    }

    // Verify intermediate state
    TS_ASSERT_EQUALS(algo.getFleetPosition(0).x,   2000);
    TS_ASSERT_EQUALS(algo.getFleetPosition(0).y,     41);

    TS_ASSERT_EQUALS(algo.getFleetPosition(1).x,  16997);
    TS_ASSERT_EQUALS(algo.getFleetPosition(1).y,    297);

    TS_ASSERT_EQUALS(algo.getFleetPosition(2).x,  14915);
    TS_ASSERT_EQUALS(algo.getFleetPosition(2).y,   2727);

    TS_ASSERT_EQUALS(algo.getFleetPosition(3).x, -18000);
    TS_ASSERT_EQUALS(algo.getFleetPosition(3).y,    374);

    TS_ASSERT_EQUALS(algo.getCrew(0),                 110);
    TS_ASSERT_EQUALS(algo.getDamage(0),                 0);
    TS_ASSERT_EQUALS(algo.getShield(0),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(0),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(0),            0);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(0), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(0),          10);

    TS_ASSERT_EQUALS(algo.getCrew(1),                1035);
    TS_ASSERT_EQUALS(algo.getDamage(1),                 0);
    TS_ASSERT_EQUALS(algo.getShield(1),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(1),   16);
    TS_ASSERT_EQUALS(algo.getNumFighters(1),           30);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(1), 2);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(1),           0);

    TS_ASSERT_EQUALS(algo.getCrew(2),                1035);
    TS_ASSERT_EQUALS(algo.getDamage(2),                 0);
    TS_ASSERT_EQUALS(algo.getShield(2),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(2),   13);
    TS_ASSERT_EQUALS(algo.getNumFighters(2),           30);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(2), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(2),         0);

    TS_ASSERT_EQUALS(algo.getCrew(3),                   0);
    TS_ASSERT_EQUALS(algo.getDamage(3),                 0);
    TS_ASSERT_EQUALS(algo.getShield(3),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(3),   26);
    TS_ASSERT_EQUALS(algo.getNumFighters(3),            6);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(3), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(3),           0);

    TS_ASSERT_EQUALS(algo.getCrew(4),                2054);
    TS_ASSERT_EQUALS(algo.getDamage(4),                 0);
    TS_ASSERT_EQUALS(algo.getShield(4),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(4),   32);
    TS_ASSERT_EQUALS(algo.getNumFighters(4),           73);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(4), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(4),           0);

    TS_ASSERT_EQUALS(algo.getCrew(5),                2054);
    TS_ASSERT_EQUALS(algo.getDamage(5),                 0);
    TS_ASSERT_EQUALS(algo.getShield(5),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(5),   32);
    TS_ASSERT_EQUALS(algo.getNumFighters(5),           63);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(5), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(5),         0);

    TS_ASSERT_EQUALS(algo.getCrew(6),                 787);
    TS_ASSERT_EQUALS(algo.getDamage(6),                 0);
    TS_ASSERT_EQUALS(algo.getShield(6),                46);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(6),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(6),            0);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(6), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(6),          89);

    TS_ASSERT_EQUALS(algo.getCrew(7),                 787);
    TS_ASSERT_EQUALS(algo.getDamage(7),                 0);
    TS_ASSERT_EQUALS(algo.getShield(7),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(7),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(7),            0);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(7), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(7),         100);

    // Play to end
    while (algo.playCycle(env.env, vis))
        ;

    // Verify end state
    TS_ASSERT_EQUALS(algo.getTime(), 352);

    TS_ASSERT_EQUALS(algo.getShipId(0),         43);
    TS_ASSERT_EQUALS(algo.getDamage(0),          0);
    TS_ASSERT_EQUALS(algo.getCrew(0),          110);
    TS_ASSERT_EQUALS(algo.getShield(0),         35);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(0),    7);
    TS_ASSERT_EQUALS(algo.getNumFighters(0),     0);

    TS_ASSERT_EQUALS(algo.getShipId(1),        201);
    TS_ASSERT_EQUALS(algo.getDamage(1),          0);
    TS_ASSERT_EQUALS(algo.getCrew(1),         1035);
    TS_ASSERT_EQUALS(algo.getShield(1),        100);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(1),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(1),    50);

    TS_ASSERT_EQUALS(algo.getShipId(2),        310);
    TS_ASSERT_EQUALS(algo.getDamage(2),        105);
    TS_ASSERT_EQUALS(algo.getCrew(2),          971);
    TS_ASSERT_EQUALS(algo.getShield(2),          0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(2),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(2),    32);

    TS_ASSERT_EQUALS(algo.getShipId(3),        442);
    TS_ASSERT_EQUALS(algo.getDamage(3),          0);
    TS_ASSERT_EQUALS(algo.getCrew(3),            0);
    TS_ASSERT_EQUALS(algo.getShield(3),        100);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(3),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(3),    27);

    TS_ASSERT_EQUALS(algo.getShipId(4),        692);
    TS_ASSERT_EQUALS(algo.getDamage(4),          0);
    TS_ASSERT_EQUALS(algo.getCrew(4),         2054);
    TS_ASSERT_EQUALS(algo.getShield(4),        100);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(4),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(4),   105);

    TS_ASSERT_EQUALS(algo.getShipId(5),        974);
    TS_ASSERT_EQUALS(algo.getDamage(5),         63);
    TS_ASSERT_EQUALS(algo.getCrew(5),         2010);
    TS_ASSERT_EQUALS(algo.getShield(5),          0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(5),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(5),    95);

    TS_ASSERT_EQUALS(algo.getShipId(6),        406);
    TS_ASSERT_EQUALS(algo.getDamage(6),        103);
    TS_ASSERT_EQUALS(algo.getCrew(6),          646);
    TS_ASSERT_EQUALS(algo.getShield(6),          0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(6),   76);
    TS_ASSERT_EQUALS(algo.getNumFighters(6),     0);

    TS_ASSERT_EQUALS(algo.getShipId(7),        721);
    TS_ASSERT_EQUALS(algo.getDamage(7),        100);
    TS_ASSERT_EQUALS(algo.getCrew(7),          629);
    TS_ASSERT_EQUALS(algo.getShield(7),          0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(7),   94);
    TS_ASSERT_EQUALS(algo.getNumFighters(7),     0);
}

/** Test playback, non-AC.
    A: load a buffer. Disable AllowAlternativeCombat. Play it.
    E: check against results from PCC2 implementation. */
void
TestGameVcrFlakAlgorithm::testPlayNonAC()
{
    // Environment
    TestEnvironment env;
    afl::string::NullTranslator tx;
    initConfig(env);
    initBeams(env);
    initTorpedoes(env);
    env.config[game::config::HostConfiguration::AllowAlternativeCombat].set(0);

    // Test
    game::vcr::flak::Setup testee;
    afl::charset::Utf8Charset cs;
    testee.load("testPlayNonAC", FILE_CONTENT, cs, tx);

    game::vcr::flak::NullVisualizer vis;
    game::vcr::flak::Algorithm algo(testee, env.env);
    algo.init(env.env, vis);

    // Play to time 100
    while (algo.getTime() < 100) {
        TS_ASSERT(algo.playCycle(env.env, vis));
    }

    // Verify intermediate state
    TS_ASSERT_EQUALS(algo.getFleetPosition(0).x,   2000);
    TS_ASSERT_EQUALS(algo.getFleetPosition(0).y,     41);

    TS_ASSERT_EQUALS(algo.getFleetPosition(1).x,  16997);
    TS_ASSERT_EQUALS(algo.getFleetPosition(1).y,    297);

    TS_ASSERT_EQUALS(algo.getFleetPosition(2).x,  14915);
    TS_ASSERT_EQUALS(algo.getFleetPosition(2).y,   2727);

    TS_ASSERT_EQUALS(algo.getFleetPosition(3).x, -18000);
    TS_ASSERT_EQUALS(algo.getFleetPosition(3).y,    374);

    TS_ASSERT_EQUALS(algo.getCrew(0),                 110);
    TS_ASSERT_EQUALS(algo.getDamage(0),                 0);
    TS_ASSERT_EQUALS(algo.getShield(0),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(0),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(0),            0);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(0), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(0),          10);

    TS_ASSERT_EQUALS(algo.getCrew(1),                1035);
    TS_ASSERT_EQUALS(algo.getDamage(1),                 0);
    TS_ASSERT_EQUALS(algo.getShield(1),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(1),   16);
    TS_ASSERT_EQUALS(algo.getNumFighters(1),           30);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(1), 2);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(1),           0);

    TS_ASSERT_EQUALS(algo.getCrew(2),                1035);
    TS_ASSERT_EQUALS(algo.getDamage(2),                 0);
    TS_ASSERT_EQUALS(algo.getShield(2),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(2),   13);
    TS_ASSERT_EQUALS(algo.getNumFighters(2),           30);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(2), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(2),           0);

    TS_ASSERT_EQUALS(algo.getCrew(3),                   0);
    TS_ASSERT_EQUALS(algo.getDamage(3),                 0);
    TS_ASSERT_EQUALS(algo.getShield(3),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(3),   26);
    TS_ASSERT_EQUALS(algo.getNumFighters(3),            6);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(3), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(3),           0);

    TS_ASSERT_EQUALS(algo.getCrew(4),                2054);
    TS_ASSERT_EQUALS(algo.getDamage(4),                 0);
    TS_ASSERT_EQUALS(algo.getShield(4),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(4),   32);
    TS_ASSERT_EQUALS(algo.getNumFighters(4),           73);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(4), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(4),           0);

    TS_ASSERT_EQUALS(algo.getCrew(5),                2054);
    TS_ASSERT_EQUALS(algo.getDamage(5),                 0);
    TS_ASSERT_EQUALS(algo.getShield(5),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(5),   32);
    TS_ASSERT_EQUALS(algo.getNumFighters(5),           63);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(5), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(5),           0);

    TS_ASSERT_EQUALS(algo.getCrew(6),                 787);
    TS_ASSERT_EQUALS(algo.getDamage(6),                 0);
    TS_ASSERT_EQUALS(algo.getShield(6),                21);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(6),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(6),            0);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(6), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(6),          89);

    TS_ASSERT_EQUALS(algo.getCrew(7),                 787);
    TS_ASSERT_EQUALS(algo.getDamage(7),                 0);
    TS_ASSERT_EQUALS(algo.getShield(7),               100);
    TS_ASSERT_EQUALS(algo.getNumFightersLaunched(7),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(7),            0);
    TS_ASSERT_EQUALS(algo.getFighterLaunchCountdown(7), 0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(7),         100);

    // Play to end
    while (algo.playCycle(env.env, vis))
        ;

    // Verify end state
    TS_ASSERT_EQUALS(algo.getTime(), 244);

    TS_ASSERT_EQUALS(algo.getShipId(0),         43);
    TS_ASSERT_EQUALS(algo.getDamage(0),          0);
    TS_ASSERT_EQUALS(algo.getCrew(0),          110);
    TS_ASSERT_EQUALS(algo.getShield(0),        100);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(0),    9);
    TS_ASSERT_EQUALS(algo.getNumFighters(0),     0);

    TS_ASSERT_EQUALS(algo.getShipId(1),        201);
    TS_ASSERT_EQUALS(algo.getDamage(1),          0);
    TS_ASSERT_EQUALS(algo.getCrew(1),         1035);
    TS_ASSERT_EQUALS(algo.getShield(1),        100);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(1),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(1),    46);

    TS_ASSERT_EQUALS(algo.getShipId(2),        310);
    TS_ASSERT_EQUALS(algo.getDamage(2),          0);
    TS_ASSERT_EQUALS(algo.getCrew(2),         1035);
    TS_ASSERT_EQUALS(algo.getShield(2),        100);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(2),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(2),    43);

    TS_ASSERT_EQUALS(algo.getShipId(3),        442);
    TS_ASSERT_EQUALS(algo.getDamage(3),          0);
    TS_ASSERT_EQUALS(algo.getCrew(3),            0);
    TS_ASSERT_EQUALS(algo.getShield(3),        100);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(3),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(3),    32);

    TS_ASSERT_EQUALS(algo.getShipId(4),        692);
    TS_ASSERT_EQUALS(algo.getDamage(4),          0);
    TS_ASSERT_EQUALS(algo.getCrew(4),         2054);
    TS_ASSERT_EQUALS(algo.getShield(4),        100);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(4),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(4),   105);

    TS_ASSERT_EQUALS(algo.getShipId(5),        974);
    TS_ASSERT_EQUALS(algo.getDamage(5),         26);
    TS_ASSERT_EQUALS(algo.getCrew(5),         1976);
    TS_ASSERT_EQUALS(algo.getShield(5),          0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(5),    0);
    TS_ASSERT_EQUALS(algo.getNumFighters(5),    95);

    TS_ASSERT_EQUALS(algo.getShipId(6),        406);
    TS_ASSERT_EQUALS(algo.getDamage(6),        117);
    TS_ASSERT_EQUALS(algo.getCrew(6),          787);
    TS_ASSERT_EQUALS(algo.getShield(6),          0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(6),   80);
    TS_ASSERT_EQUALS(algo.getNumFighters(6),     0);

    TS_ASSERT_EQUALS(algo.getShipId(7),        721);
    TS_ASSERT_EQUALS(algo.getDamage(7),        103);
    TS_ASSERT_EQUALS(algo.getCrew(7),          761);
    TS_ASSERT_EQUALS(algo.getShield(7),          0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(7),  115);
    TS_ASSERT_EQUALS(algo.getNumFighters(7),     0);
}

/** Test setup of a simple mixed battle.
    A: set up a battle.
    E: verify result (regression test). */
void
TestGameVcrFlakAlgorithm::testSetup()
{
    // Environment
    TestEnvironment env;
    afl::string::NullTranslator tx;
    initConfig(env);
    initBeams(env);
    initTorpedoes(env);
    game::vcr::flak::Configuration config;

    // Test
    game::vcr::flak::Setup testee;
    afl::charset::Utf8Charset cs; // FIXME

    // - a Klingon warship
    game::vcr::flak::Setup::FleetIndex_t fleet1 = testee.addFleet(4);
    TS_ASSERT_EQUALS(fleet1, 0U);
    game::vcr::flak::Object ship1;
    ship1.setCrew(100);
    ship1.setId(10);
    ship1.setOwner(4);
    ship1.setHull(1);
    ship1.setNumBeams(4);
    ship1.setBeamType(10);
    ship1.setNumLaunchers(3);
    ship1.setNumTorpedoes(20);
    ship1.setTorpedoType(8);
    ship1.setMass(300);
    ship1.init(config);
    testee.addShip(ship1);

    // - a Klingon freighter
    game::vcr::flak::Setup::FleetIndex_t fleet2 = testee.addFleet(4);
    TS_ASSERT_EQUALS(fleet2, 1U);
    game::vcr::flak::Object ship2;
    ship2.setCrew(100);
    ship2.setId(20);
    ship2.setOwner(4);
    ship2.setHull(2);
    ship2.setMass(100);
    ship2.init(config);
    testee.addShip(ship2);

    // - a Fed planet
    game::vcr::flak::Setup::FleetIndex_t fleet3 = testee.addFleet(1);
    TS_ASSERT_EQUALS(fleet3, 2U);
    game::vcr::flak::Object planet3;
    planet3.setCrew(0);
    planet3.setId(444);
    planet3.setOwner(1);
    planet3.setHull(0);
    planet3.setNumBeams(6);
    planet3.setBeamType(6);
    planet3.setNumBays(5);
    planet3.setNumFighters(15);
    planet3.setMass(150);
    planet3.setIsPlanet(true);
    planet3.init(config);
    testee.addShip(planet3);

    // Attack lists
    testee.startAttackList(0);
    testee.addAttackListEntry(2, 10);
    testee.endAttackList(0);

    testee.startAttackList(2);
    testee.addAttackListEntry(0, 10);
    testee.endAttackList(2);

    TS_ASSERT_EQUALS(testee.getNumShips(), 3U);
    TS_ASSERT_EQUALS(testee.getNumFleets(), 3U);

    // Prepare
    util::RandomNumberGenerator rng(1);
    testee.initAfterSetup(config, env.env, rng);

    // Verify:
    // - freighter has been remvoed
    TS_ASSERT_EQUALS(testee.getNumShips(), 2U);
    TS_ASSERT_EQUALS(testee.getNumFleets(), 2U);

    // - check locations
    TS_ASSERT_EQUALS(testee.getFleetByIndex(0).x, -28000);  // StartingDistanceShip + 2*StartingDistancePerPlayer
    TS_ASSERT_EQUALS(testee.getFleetByIndex(0).y, 0);
    TS_ASSERT_EQUALS(testee.getFleetByIndex(1).x, 12000);   // StartingDistancePlanet + 2*StartingDistancePerPlayer
    TS_ASSERT_EQUALS(testee.getFleetByIndex(1).y, 0);

    // Run it; verify result
    testee.setSeed(12345);

    game::vcr::flak::NullVisualizer vis;
    game::vcr::flak::Algorithm algo(testee, env.env);
    algo.init(env.env, vis);

    while (algo.playCycle(env.env, vis))
        ;

    TS_ASSERT_EQUALS(algo.getTime(), 234);

    TS_ASSERT_EQUALS(algo.getFleetPosition(0).x,  -4800);
    TS_ASSERT_EQUALS(algo.getFleetPosition(0).y,      0);

    TS_ASSERT_EQUALS(algo.getFleetPosition(1).x,  12000);
    TS_ASSERT_EQUALS(algo.getFleetPosition(1).y,      0);

    TS_ASSERT_EQUALS(algo.getDamage(0),       37);
    TS_ASSERT_EQUALS(algo.getCrew(0),         47);
    TS_ASSERT_EQUALS(algo.getShield(0),        0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(0), 11);
    TS_ASSERT_EQUALS(algo.getNumFighters(0),   0);

    TS_ASSERT_EQUALS(algo.getDamage(1),      107);
    TS_ASSERT_EQUALS(algo.getCrew(1),          0);
    TS_ASSERT_EQUALS(algo.getShield(1),        0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(1),  0);
    TS_ASSERT_EQUALS(algo.getNumFighters(1),   1);
}

/** Test setup of a simple battle involving fighters.
    A: set up a battle.
    E: verify result (regression test). */
void
TestGameVcrFlakAlgorithm::testSetupFighters()
{
    // Environment
    TestEnvironment env;
    afl::string::NullTranslator tx;
    initConfig(env);
    initBeams(env);
    initTorpedoes(env);
    game::vcr::flak::Configuration config;

    // We want to check fighter intercept!
    env.config[game::config::HostConfiguration::FighterKillOdds].set(30);

    // Test
    game::vcr::flak::Setup testee;
    afl::charset::Utf8Charset cs; // FIXME

    // - a small carrier
    game::vcr::flak::Setup::FleetIndex_t fleet1 = testee.addFleet(6);
    TS_ASSERT_EQUALS(fleet1, 0U);
    game::vcr::flak::Object ship1;
    ship1.setCrew(100);
    ship1.setId(10);
    ship1.setOwner(6);
    ship1.setHull(1);
    ship1.setNumBeams(4);
    ship1.setBeamType(10);
    ship1.setNumBays(10);
    ship1.setNumFighters(20);
    ship1.setMass(100);
    ship1.init(config);
    testee.addShip(ship1);

    // - a larger carrier
    game::vcr::flak::Setup::FleetIndex_t fleet2 = testee.addFleet(6);
    TS_ASSERT_EQUALS(fleet2, 1U);
    game::vcr::flak::Object ship2;
    ship2.setCrew(100);
    ship2.setId(10);
    ship2.setOwner(6);
    ship2.setHull(2);
    ship2.setNumBeams(4);
    ship2.setBeamType(10);
    ship2.setNumBays(10);
    ship2.setNumFighters(200);
    ship2.setMass(800);
    ship2.init(config);
    testee.addShip(ship2);

    // - another carrier, enemy
    game::vcr::flak::Setup::FleetIndex_t fleet3 = testee.addFleet(10);
    TS_ASSERT_EQUALS(fleet3, 2U);
    game::vcr::flak::Object ship3;
    ship3.setCrew(100);
    ship3.setId(10);
    ship3.setOwner(10);
    ship3.setHull(3);
    ship3.setNumBeams(4);
    ship3.setBeamType(10);
    ship3.setNumBays(8);
    ship3.setNumFighters(200);
    ship3.setMass(400);
    ship3.init(config);
    testee.addShip(ship3);

    // Attack lists
    testee.startAttackList(fleet1);
    testee.addAttackListEntry(2, 10);
    testee.endAttackList(fleet1);

    testee.startAttackList(fleet2);
    testee.addAttackListEntry(2, 12);
    testee.endAttackList(fleet2);

    testee.startAttackList(fleet3);
    testee.addAttackListEntry(1, 10);
    testee.addAttackListEntry(0, 5);
    testee.endAttackList(fleet3);

    TS_ASSERT_EQUALS(testee.getNumShips(), 3U);
    TS_ASSERT_EQUALS(testee.getNumFleets(), 3U);

    // Prepare
    util::RandomNumberGenerator rng(1);
    testee.initAfterSetup(config, env.env, rng);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumShips(), 3U);
    TS_ASSERT_EQUALS(testee.getNumFleets(), 3U);

    // - check locations
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(0).x), 28000);    // SDShip + 2*SDPPlayer
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(0).y), 0);
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(1).x), 32995);    // SDShip + 2*SDPPlayer + SDPFleet (approx)
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(1).y), 576);
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(2).x), -28000);   // -(SDShip + 2*SDPPlayer)
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(2).y), 0);

    // Run it; verify result
    testee.setSeed(12345);

    game::vcr::flak::NullVisualizer vis;
    game::vcr::flak::Algorithm algo(testee, env.env);
    algo.init(env.env, vis);

    while (algo.playCycle(env.env, vis))
        ;

    TS_ASSERT_EQUALS(algo.getTime(), 285);

    TS_ASSERT_EQUALS(algo.getFleetPosition(0).x,   5000);
    TS_ASSERT_EQUALS(algo.getFleetPosition(0).y,      0);

    TS_ASSERT_EQUALS(algo.getFleetPosition(1).x,   8295);
    TS_ASSERT_EQUALS(algo.getFleetPosition(1).y,    252);

    TS_ASSERT_EQUALS(algo.getFleetPosition(2).x,  -3900);
    TS_ASSERT_EQUALS(algo.getFleetPosition(2).y,     22);

    TS_ASSERT_EQUALS(algo.getDamage(0),       68);
    TS_ASSERT_EQUALS(algo.getCrew(0),          0);
    TS_ASSERT_EQUALS(algo.getShield(0),        0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(0),  0);
    TS_ASSERT_EQUALS(algo.getNumFighters(0),   1);

    TS_ASSERT_EQUALS(algo.getDamage(1),        0);
    TS_ASSERT_EQUALS(algo.getCrew(1),        100);
    TS_ASSERT_EQUALS(algo.getShield(1),        0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(1),  0);
    TS_ASSERT_EQUALS(algo.getNumFighters(1), 200);

    TS_ASSERT_EQUALS(algo.getDamage(2),       66);
    TS_ASSERT_EQUALS(algo.getCrew(2),          0);
    TS_ASSERT_EQUALS(algo.getShield(2),        0);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(2),  0);
    TS_ASSERT_EQUALS(algo.getNumFighters(2), 160);
}

/** Test cloning status.
    A: set up a battle. Create and clone status tokens. Create and clone setup.
    E: verify all results (regression test). */
void
TestGameVcrFlakAlgorithm::testCloneStatus()
{
    // Environment
    TestEnvironment env;
    afl::string::NullTranslator tx;
    initConfig(env);
    initBeams(env);
    initTorpedoes(env);

    // Test
    game::vcr::flak::Setup testee;
    afl::charset::Utf8Charset cs;
    testee.load("testCloneStatus", FILE_CONTENT, cs, tx);

    // Create a copy of the battle
    game::vcr::flak::Setup copy(testee);

    // Play to time 100 -- up to here, same as testPlay()
    game::vcr::flak::NullVisualizer vis;
    game::vcr::flak::Algorithm algo(testee, env.env);
    algo.init(env.env, vis);

    while (algo.getTime() < 100) {
        TS_ASSERT(algo.playCycle(env.env, vis));
    }

    // Create a status token
    std::auto_ptr<game::vcr::flak::Algorithm::StatusToken> tok(algo.createStatusToken());

    // // Clone the token
    // std::auto_ptr<game::vcr::flak::Algorithm::StatusToken> tok2(new game::vcr::flak::Algorithm::StatusToken(*tok));

    // Complete the original
    while (algo.playCycle(env.env, vis))
        ;
    TS_ASSERT_EQUALS(algo.getTime(), 352);
    TS_ASSERT_EQUALS(algo.getDamage(6),   103);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(6),   76);

    // Complete the copy
    game::vcr::flak::NullVisualizer copyVis;
    game::vcr::flak::Algorithm copyAlgo(copy, env.env);
    copyAlgo.init(env.env, copyVis);
    while (copyAlgo.playCycle(env.env, copyVis))
        ;
    TS_ASSERT_EQUALS(copyAlgo.getTime(), 352);
    TS_ASSERT_EQUALS(copyAlgo.getDamage(6),      103);
    TS_ASSERT_EQUALS(copyAlgo.getNumTorpedoes(6), 76);

    // Rewind to status token and complete
    tok->storeTo(algo);
    while (algo.playCycle(env.env, vis))
        ;
    TS_ASSERT_EQUALS(algo.getTime(), 352);
    TS_ASSERT_EQUALS(algo.getDamage(6),      103);
    TS_ASSERT_EQUALS(algo.getNumTorpedoes(6), 76);

    // Rewind to second status token and complete
    // tok2->storeTo(algo);
    // while (algo.playCycle(env.env, vis))
    //     ;
    // TS_ASSERT_EQUALS(algo.getTime(), 352);
    // TS_ASSERT_EQUALS(algo.getDamage(6),   103);
    // TS_ASSERT_EQUALS(algo.getNumTorpedoes(6),   76);
}

/** Test setup of a battle involving capture-back.
    A: set up a battle with one freighter, one small warship, and a large warship.
    E: verify result (regression test): small warship captures freighter, large warship destroys small warship and therefore captures back. */
void
TestGameVcrFlakAlgorithm::testSetupCapture()
{
    // Environment
    TestEnvironment env;
    afl::string::NullTranslator tx;
    initConfig(env);
    initBeams(env);
    initTorpedoes(env);
    game::vcr::flak::Configuration config;

    // Test
    game::vcr::flak::Setup testee;
    afl::charset::Utf8Charset cs; // FIXME

    // - a freighter
    game::vcr::flak::Setup::FleetIndex_t fleet1 = testee.addFleet(6);
    TS_ASSERT_EQUALS(fleet1, 0U);
    game::vcr::flak::Object ship1;
    ship1.setCrew(2);
    ship1.setId(10);
    ship1.setOwner(6);
    ship1.setHull(1);
    ship1.setMass(800);
    ship1.init(config);
    testee.addShip(ship1);

    // - a large warship
    game::vcr::flak::Setup::FleetIndex_t fleet2 = testee.addFleet(6);
    TS_ASSERT_EQUALS(fleet2, 1U);
    game::vcr::flak::Object ship2;
    ship2.setCrew(100);
    ship2.setId(20);
    ship2.setOwner(6);
    ship2.setHull(2);
    ship2.setNumBeams(10);
    ship2.setBeamType(10);
    ship2.setMass(800);
    ship2.init(config);
    testee.addShip(ship2);

    // - an enemy ship with anti-crew beams
    game::vcr::flak::Setup::FleetIndex_t fleet3 = testee.addFleet(10);
    TS_ASSERT_EQUALS(fleet3, 2U);
    game::vcr::flak::Object ship3;
    ship3.setCrew(100);
    ship3.setId(10);
    ship3.setOwner(10);
    ship3.setHull(3);
    ship3.setNumBeams(10);
    ship3.setBeamType(9);
    ship3.setMass(100);
    ship3.init(config);
    testee.addShip(ship3);

    // Attack lists
    testee.addAttackListEntry(2, 10);
    testee.getFleetByIndex(fleet1).firstAttackListIndex = 0;
    testee.getFleetByIndex(fleet1).numAttackListEntries = 1;
    testee.addAttackListEntry(2, 12);
    testee.getFleetByIndex(fleet2).firstAttackListIndex = 1;
    testee.getFleetByIndex(fleet2).numAttackListEntries = 1;
    testee.addAttackListEntry(1, 10);
    testee.addAttackListEntry(0, 5);
    testee.getFleetByIndex(fleet3).firstAttackListIndex = 2;
    testee.getFleetByIndex(fleet3).numAttackListEntries = 2;

    TS_ASSERT_EQUALS(testee.getNumShips(), 3U);
    TS_ASSERT_EQUALS(testee.getNumFleets(), 3U);

    // Prepare
    util::RandomNumberGenerator rng(1);
    testee.initAfterSetup(config, env.env, rng);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumShips(), 3U);
    TS_ASSERT_EQUALS(testee.getNumFleets(), 3U);

    // - check locations
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(0).x), 28000);    // SDShip + 2*SDPPlayer
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(0).y), 0);
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(1).x), 32995);    // SDShip + 2*SDPPlayer + SDPFleet (approx)
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(1).y), 576);
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(2).x), -28000);   // -(SDShip + 2*SDPPlayer)
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(2).y), 0);

    // Run it; verify result
    testee.setSeed(12345);

    game::vcr::flak::NullVisualizer vis;
    game::vcr::flak::Algorithm algo(testee, env.env);
    algo.init(env.env, vis);

    while (algo.playCycle(env.env, vis))
        ;

    TS_ASSERT_EQUALS(algo.getTime(), 358);

    TS_ASSERT_EQUALS(algo.getDamage(0),     0);
    TS_ASSERT_EQUALS(algo.getCrew(0),       0);
    TS_ASSERT_EQUALS(algo.getShield(0),     0);

    TS_ASSERT_EQUALS(algo.getDamage(1),     4);
    TS_ASSERT_EQUALS(algo.getCrew(1),      85);
    TS_ASSERT_EQUALS(algo.getShield(1),     0);

    TS_ASSERT_EQUALS(algo.getDamage(2),    99);
    TS_ASSERT_EQUALS(algo.getCrew(2),      25);
    TS_ASSERT_EQUALS(algo.getShield(2),     0);

    // Determine captors
    size_t captorIndex;
    TS_ASSERT_EQUALS(algo.findCaptor(0, rng).get(captorIndex), true);
    TS_ASSERT_EQUALS(captorIndex, 1U);
}

/** Test setup of a battle involving death-ray capture.
    A: set up a battle with two ships, one of which with death-ray torpedoes.
    E: verify result (regression test). */
void
TestGameVcrFlakAlgorithm::testSetupCaptureDeathRay()
{
    // Environment
    TestEnvironment env;
    afl::string::NullTranslator tx;
    initConfig(env);
    initBeams(env);
    initTorpedoes(env);
    env.torps.get(9)->setDamagePower(0);            // #9 is a death-ray torp
    game::vcr::flak::Configuration config;

    // Test
    game::vcr::flak::Setup testee;
    afl::charset::Utf8Charset cs; // FIXME

    // - a freighter
    game::vcr::flak::Setup::FleetIndex_t fleet1 = testee.addFleet(4);
    TS_ASSERT_EQUALS(fleet1, 0U);
    game::vcr::flak::Object ship1;
    ship1.setCrew(100);
    ship1.setId(10);
    ship1.setOwner(4);
    ship1.setHull(1);
    ship1.setMass(800);
    ship1.setShield(100);
    ship1.init(config);
    testee.addShip(ship1);

    // - enemy with death-ray torps
    game::vcr::flak::Setup::FleetIndex_t fleet2 = testee.addFleet(5);
    TS_ASSERT_EQUALS(fleet2, 1U);
    game::vcr::flak::Object ship2;
    ship2.setCrew(100);
    ship2.setId(20);
    ship2.setOwner(5);
    ship2.setHull(2);
    ship2.setNumLaunchers(10);
    ship2.setTorpedoType(9);
    ship2.setNumTorpedoes(999);
    ship2.setMass(400);
    ship2.init(config);
    testee.addShip(ship2);

    // Attack lists
    testee.addAttackListEntry(1, 10);
    testee.getFleetByIndex(fleet1).firstAttackListIndex = 0;
    testee.getFleetByIndex(fleet1).numAttackListEntries = 1;
    testee.addAttackListEntry(0, 10);
    testee.getFleetByIndex(fleet2).firstAttackListIndex = 1;
    testee.getFleetByIndex(fleet2).numAttackListEntries = 1;

    TS_ASSERT_EQUALS(testee.getNumShips(), 2U);
    TS_ASSERT_EQUALS(testee.getNumFleets(), 2U);

    // Prepare
    util::RandomNumberGenerator rng(1);
    testee.initAfterSetup(config, env.env, rng);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumShips(), 2U);
    TS_ASSERT_EQUALS(testee.getNumFleets(), 2U);

    // - check locations
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(0).x), 28000);    // SDShip + 2*SDPPlayer
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(0).y), 0);
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(1).x), -28000);   // -(SDShip + 2*SDPPlayer)
    TS_ASSERT_EQUALS(int(testee.getFleetByIndex(1).y), 0);

    // Run it; verify result
    testee.setSeed(77777);

    game::vcr::flak::NullVisualizer vis;
    game::vcr::flak::Algorithm algo(testee, env.env);
    algo.init(env.env, vis);

    while (algo.playCycle(env.env, vis))
        ;

    TS_ASSERT_EQUALS(algo.getTime(), 510);

    TS_ASSERT_EQUALS(algo.getDamage(0),         0);
    TS_ASSERT_EQUALS(algo.getCrew(0),           0);
    TS_ASSERT_EQUALS(algo.getShield(0),       100);

    TS_ASSERT_EQUALS(algo.getDamage(1),         0);
    TS_ASSERT_EQUALS(algo.getCrew(1),         100);
    TS_ASSERT_EQUALS(algo.getShield(1),         0);
    TS_ASSERT_EQUALS(int(algo.getNumTorpedoes(1)), 939);

    // Determine captors
    size_t captorIndex;
    TS_ASSERT_EQUALS(algo.findCaptor(0, rng).get(captorIndex), true);
    TS_ASSERT_EQUALS(captorIndex, 1U);
}

/** Test a 1:1 fight with all player combinations.
    This fight contains a Cube vs MDSF fight that is decided as capture-by-torpedoes.
    The result therefore is always the same, because the Lizard 150% damage bonus and
    the Privateer 3x beam-kill bonus are not applied.
    A: load a fight. Set player combinations.
    E: verify same result for all, cross-checked with original server result */
void
TestGameVcrFlakAlgorithm::testPair()
{
    // Environment
    afl::string::NullTranslator tx;
    game::config::HostConfiguration config; // default
    game::spec::ShipList shipList;
    game::test::initStandardTorpedoes(shipList);
    game::test::initStandardBeams(shipList);
    game::vcr::flak::GameEnvironment env(config, shipList.beams(), shipList.launchers());

    // Test
    for (int left = 1; left <= 12; ++left) {
        for (int right = 1; right <= 12; ++right) {
            // Name the test case
            String_t label = afl::string::Format("%d vs %d", left, right);
            if (left == right) {
                continue;
            }

            // Load template
            game::vcr::flak::Setup testee;
            afl::charset::Utf8Charset cs;
            testee.load("testPair", ONE_ON_ONE_CONTENT, cs, tx);

            // Override ship owners
            TS_ASSERT_EQUALS(testee.getNumShips(), 2U);
            testee.getShipByIndex(0).setOwner(left);
            testee.getShipByIndex(1).setOwner(right);
            testee.getFleetByIndex(0).player = left;
            testee.getFleetByIndex(1).player = right;

            game::vcr::flak::NullVisualizer vis;
            game::vcr::flak::Algorithm algo(testee, env);
            algo.init(env, vis);

            // Play to end
            while (algo.playCycle(env, vis))
                ;

            // Verify end state
            TSM_ASSERT_EQUALS(label.c_str(), algo.getTime(), 241);

            TSM_ASSERT_EQUALS(label.c_str(), algo.getShipId(0),       100);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getDamage(0),         0);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getCrew(0),         102);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getShield(0),       100);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getNumTorpedoes(0),  48);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getNumFighters(0),    0);

            TSM_ASSERT_EQUALS(label.c_str(), algo.getShipId(1),       200);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getDamage(1),        63);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getCrew(1),           0);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getShield(1),         0);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getNumTorpedoes(1),   0);
            TSM_ASSERT_EQUALS(label.c_str(), algo.getNumFighters(1),    0);
        }
    }
}
