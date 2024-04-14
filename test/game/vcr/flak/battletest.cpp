/**
  *  \file test/game/vcr/flak/battletest.cpp
  *  \brief Test for game::vcr::flak::Battle
  */

#include "game/vcr/flak/battle.hpp"

#include "afl/base/countof.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/componentvector.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "game/vcr/flak/algorithm.hpp"
#include "game/vcr/flak/configuration.hpp"
#include "game/vcr/flak/gameenvironment.hpp"
#include "game/vcr/flak/nullvisualizer.hpp"
#include "game/vcr/flak/setup.hpp"
#include "game/vcr/score.hpp"

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

    void initConfig(game::config::HostConfiguration& config)
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
            { "AllowEngineShieldBonus",   "Yes" },
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
            config.setOption(OPTIONS[i][0], OPTIONS[i][1], game::config::ConfigurationOption::Game);
        }
    }

    void initBeams(game::spec::ShipList& list)
    {
        // Beams from game FLAK0
        //                            Las KOZ Dis Pha Dis ERa Ion TlB Inp MtS
        static const int KILL[]   = {  1, 10,  7, 15, 40, 20, 10, 45, 70, 40 };
        static const int DAMAGE[] = {  3,  1, 10, 25, 10, 40, 60, 55, 35, 80 };
        for (int i = 1; i <= 10; ++i) {
            game::spec::Beam* b = list.beams().create(i);
            b->setKillPower(KILL[i-1]);
            b->setDamagePower(DAMAGE[i-1]);
        }
    }

    void initTorpedoes(game::spec::ShipList& list)
    {
        // Torpedoes from game FLAK0
        //                            SpR PMB FuB InB PhT Gra Ark AmB Kat SFD
        static const int KILL[]   = { 10, 60, 25, 60, 15, 30, 60, 25, 80, 50 };
        static const int DAMAGE[] = { 25,  3, 50, 20, 82, 75, 50, 90, 40, 99 };
        for (int i = 1; i <= 10; ++i) {
            game::spec::TorpedoLauncher* tl = list.launchers().create(i);
            tl->setKillPower(KILL[i-1]);
            tl->setDamagePower(DAMAGE[i-1]);
        }
    }
}

/** Simple functionality test */
AFL_TEST("game.vcr.flak.Battle", a)
{
    // Environment
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    afl::string::NullTranslator tx;
    initConfig(config);
    initBeams(shipList);
    initTorpedoes(shipList);

    // Setup
    std::auto_ptr<game::vcr::flak::Setup> setup(new game::vcr::flak::Setup());
    afl::charset::Utf8Charset cs;
    setup->load("testIt", FILE_CONTENT, cs, tx);

    game::vcr::flak::Battle testee(setup);

    // Verify content
    // - getNumObjects
    a.checkEqual("01. getNumObjects", testee.getNumObjects(), 8U);

    // - getObject (before)
    a.checkEqual("11. getId", testee.getObject(0, false)->getId(), 43);
    a.checkEqual("12. getId", testee.getObject(1, false)->getId(), 201);
    a.checkEqual("13. getId", testee.getObject(2, false)->getId(), 310);
    a.checkEqual("14. getId", testee.getObject(7, false)->getId(), 721);

    a.checkEqual("21. getOwner", testee.getObject(0, false)->getOwner(), 9);
    a.checkEqual("22. getOwner", testee.getObject(1, false)->getOwner(), 9);
    a.checkEqual("23. getOwner", testee.getObject(2, false)->getOwner(), 9);
    a.checkEqual("24. getOwner", testee.getObject(7, false)->getOwner(), 4);

    a.checkEqual("31. getShield", testee.getObject(0, false)->getShield(), 100);
    a.checkEqual("32. getShield", testee.getObject(1, false)->getShield(), 100);
    a.checkEqual("33. getShield", testee.getObject(2, false)->getShield(), 100);
    a.checkEqual("34. getShield", testee.getObject(7, false)->getShield(), 100);

    a.checkNull("41. getObject", testee.getObject(8, false)); // out of range

    // - getNumGroups
    a.checkEqual("51. getNumGroups", testee.getNumGroups(), 4U);

    // - getGroupInfo
    a.checkEqual("61. firstObject", testee.getGroupInfo(0, config).firstObject, 0U);
    a.checkEqual("62. numObjects",  testee.getGroupInfo(0, config).numObjects, 3U);
    a.checkEqual("63. x",           testee.getGroupInfo(0, config).x, 12000);
    a.checkEqual("64. y",           testee.getGroupInfo(0, config).y, 0);
    a.checkEqual("65. owner",       testee.getGroupInfo(0, config).owner, 9);
    a.checkEqual("66. speed",       testee.getGroupInfo(0, config).speed, 100);

    a.checkEqual("71. firstObject", testee.getGroupInfo(1, config).firstObject, 3U);
    a.checkEqual("72. numObjects",  testee.getGroupInfo(1, config).numObjects, 1U);
    a.checkEqual("73. x",           testee.getGroupInfo(1, config).x, 16997);
    a.checkEqual("74. y",           testee.getGroupInfo(1, config).y, 297);
    a.checkEqual("75. owner",       testee.getGroupInfo(1, config).owner, 9);
    a.checkEqual("76. speed",       testee.getGroupInfo(1, config).speed, 0);

    a.checkEqual("81. firstObject", testee.getGroupInfo(2, config).firstObject, 4U);
    a.checkEqual("82. numObjects",  testee.getGroupInfo(2, config).numObjects, 2U);
    a.checkEqual("83. x",           testee.getGroupInfo(2, config).x, 21987);
    a.checkEqual("84. y",           testee.getGroupInfo(2, config).y, 768);
    a.checkEqual("85. owner",       testee.getGroupInfo(2, config).owner, 9);
    a.checkEqual("86. speed",       testee.getGroupInfo(2, config).speed, 100);

    a.checkEqual("91. firstObject", testee.getGroupInfo(3, config).firstObject, 6U);
    a.checkEqual("92. numObjects",  testee.getGroupInfo(3, config).numObjects, 2U);
    a.checkEqual("93. x",           testee.getGroupInfo(3, config).x, -28000);
    a.checkEqual("94. y",           testee.getGroupInfo(3, config).y, 0);
    a.checkEqual("95. owner",       testee.getGroupInfo(3, config).owner, 4);
    a.checkEqual("96. speed",       testee.getGroupInfo(3, config).speed, 100);

    // - getOutcome
    testee.prepareResult(config, shipList, game::vcr::Battle::NeedQuickOutcome);
    a.checkEqual("101. getOutcome", testee.getOutcome(config, shipList, 0), 0);
    a.checkEqual("102. getOutcome", testee.getOutcome(config, shipList, 1), 0);
    a.checkEqual("103. getOutcome", testee.getOutcome(config, shipList, 2), -1);
    a.checkEqual("104. getOutcome", testee.getOutcome(config, shipList, 7), -1);

    a.checkEqual("111. getOutcome", testee.getOutcome(config, shipList, 8), 0); // out of range

    // - getPlayability
    a.checkEqual("121. getPlayability", testee.getPlayability(config, shipList), game::vcr::Battle::IsPlayable);

    // - getAlgorithmName
    a.checkEqual("131. getAlgorithmName", testee.getAlgorithmName(tx), "FLAK");

    // - isESBActive
    a.checkEqual("141. isESBActive", testee.isESBActive(config), true);

    // - getPosition
    game::map::Point pt;
    a.checkEqual("151. getPosition", testee.getPosition().get(pt), true);
    a.checkEqual("152. getX", pt.getX(), 2595);
    a.checkEqual("153. getY", pt.getY(), 2526);

    // - prepareResult/getObject (after)
    testee.prepareResult(config, shipList, game::vcr::Battle::NeedCompleteResult);
    a.checkEqual("161. getDamage", testee.getObject(0, true)->getDamage(),   0);
    a.checkEqual("162. getDamage", testee.getObject(1, true)->getDamage(),   0);
    a.checkEqual("163. getDamage", testee.getObject(2, true)->getDamage(), 105);
    a.checkEqual("164. getDamage", testee.getObject(7, true)->getDamage(), 101);

    a.checkEqual("171. getShield", testee.getObject(0, true)->getShield(),  35);
    a.checkEqual("172. getShield", testee.getObject(1, true)->getShield(), 100);
    a.checkEqual("173. getShield", testee.getObject(2, true)->getShield(),   0);
    a.checkEqual("174. getShield", testee.getObject(7, true)->getShield(),   0);

    a.checkNull("181. getObject", testee.getObject(8, true)); // out of range

    // - computeScores
    // We're destroying 2*665 = 1330 kt using 6 ships, 5 surviving. That's 266 kt destroyed per ship.
    // Using PALAggressorPointsPer10KT=2, PALAggressorKillPointsPer10KT=10, that's 1.2*1330 = 1596 kt, or 319.2 points per ship.
    // We're attacking with 120+367+482+130+801+851 = 2751
    // Using EPCombatKillScaling=800, EPCombatDamageScaling=200, we get 1330000/2751 = 483 EP.
    // Check for first two units.
    {
        game::vcr::Score s;
        a.checkEqual("191. computeScores", testee.computeScores(s, 0, config, shipList), true);
        a.checkEqual("192. getBuildMillipoints", s.getBuildMillipoints().min(), 319200);
        a.checkEqual("193. getBuildMillipoints", s.getBuildMillipoints().max(), 319200);
        a.checkEqual("194. getExperience",       s.getExperience().min(), 483);
        a.checkEqual("195. getExperience",       s.getExperience().max(), 483);
        a.checkEqual("196. getTonsDestroyed",    s.getTonsDestroyed().min(), 266);
        a.checkEqual("197. getTonsDestroyed",    s.getTonsDestroyed().max(), 266);
    }
    {
        game::vcr::Score s;
        a.checkEqual("198. computeScores", testee.computeScores(s, 1, config, shipList), true);
        a.checkEqual("199. getBuildMillipoints", s.getBuildMillipoints().min(), 319200);
        a.checkEqual("200. getBuildMillipoints", s.getBuildMillipoints().max(), 319200);
        a.checkEqual("201. getExperience",       s.getExperience().min(), 483);
        a.checkEqual("202. getExperience",       s.getExperience().max(), 483);
        a.checkEqual("203. getTonsDestroyed",    s.getTonsDestroyed().min(), 266);
        a.checkEqual("204. getTonsDestroyed",    s.getTonsDestroyed().max(), 266);
    }

    // Units #2, #7 didn't survive and therefore doesn't get any points
    {
        game::vcr::Score s;
        a.checkEqual("211. computeScores", testee.computeScores(s, 2, config, shipList), false);
        a.checkEqual("212. computeScores", testee.computeScores(s, 7, config, shipList), false);
    }

    // - getAuxiliaryInformation
    a.checkEqual("221. aiSeed",    testee.getAuxiliaryInformation(game::vcr::Battle::aiSeed).orElse(-1),    0x6D3D7AC9);
    a.checkEqual("222. aiMagic",   testee.getAuxiliaryInformation(game::vcr::Battle::aiMagic).isValid(),    false);
    a.checkEqual("223. aiType",    testee.getAuxiliaryInformation(game::vcr::Battle::aiType).isValid(),     false);
    a.checkEqual("224. aiFlags",   testee.getAuxiliaryInformation(game::vcr::Battle::aiFlags).isValid(),    false);
    a.checkEqual("225. aiAmbient", testee.getAuxiliaryInformation(game::vcr::Battle::aiAmbient).orElse(-1), 0);
}
