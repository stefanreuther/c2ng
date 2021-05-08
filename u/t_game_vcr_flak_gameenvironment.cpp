/**
  *  \file u/t_game_vcr_flak_gameenvironment.cpp
  *  \brief Test for game::vcr::flak::GameEnvironment
  */

#include "game/vcr/flak/gameenvironment.hpp"

#include "t_game_vcr_flak.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "afl/base/countof.hpp"

using game::config::HostConfiguration;
using game::vcr::flak::Environment;

void
TestGameVcrFlakGameEnvironment::testConfig()
{
    // Configuration
    HostConfiguration config;
    static const char*const OPTIONS[][2] = {
        { "AllowAlternativeCombat", "1" },
        { "StandoffDistance", "32000" },
        { "BayLaunchInterval", "40" },
        { "FighterKillOdds", "80,90,70" },
        { "BayRechargeBonus", "3" },
        { "EModBayRechargeBonus", "1,2,3,4" },
        { "BeamHitFighterCharge", "900,800" },
        { "EModBeamHitFighterCharge", "-30,-70,-90,-150" },
        { "PlayerRace", "1,1,1,4,5,5,5,5,5" },
    };
    for (size_t i = 0; i < countof(OPTIONS); ++i) {
        config.setOption(OPTIONS[i][0], OPTIONS[i][1], game::config::ConfigurationOption::Game);
    }

    // Specification (dummy)
    game::spec::BeamVector_t beams;
    game::spec::TorpedoVector_t torps;

    // Testee
    game::vcr::flak::GameEnvironment testee(config, beams, torps);

    // Verify
    // - scalars
    TS_ASSERT_EQUALS(testee.getConfiguration(Environment::AllowAlternativeCombat), 1);
    TS_ASSERT_EQUALS(testee.getConfiguration(Environment::StandoffDistance), 32000);
    // - BayLaunchInterval array
    TS_ASSERT_EQUALS(testee.getConfiguration(Environment::BayLaunchInterval, 0), 40);
    TS_ASSERT_EQUALS(testee.getConfiguration(Environment::BayLaunchInterval, 1), 40);
    TS_ASSERT_EQUALS(testee.getConfiguration(Environment::BayLaunchInterval, 10), 40);
    // - FighterKillOdds array
    TS_ASSERT_EQUALS(testee.getConfiguration(Environment::FighterKillOdds, 1), 80);
    TS_ASSERT_EQUALS(testee.getConfiguration(Environment::FighterKillOdds, 2), 90);
    TS_ASSERT_EQUALS(testee.getConfiguration(Environment::FighterKillOdds, 10), 70);
    // - BayRechargeBonus experience
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BayRechargeBonus, 0, 1), 3);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BayRechargeBonus, 1, 1), 4);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BayRechargeBonus, 2, 1), 5);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BayRechargeBonus, 2, 10), 5);
    // - BeamHitFighterCharge experience
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 0, 1), 900);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 1, 1), 870);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 2, 1), 830);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 3, 1), 810);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 4, 1), 750);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 0, 2), 800);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 1, 2), 770);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 2, 2), 730);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 3, 2), 710);
    TS_ASSERT_EQUALS(testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 4, 2), 650);
    // - PlayerRace
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(0), 0);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(1), 1);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(2), 1);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(4), 4);
}

void
TestGameVcrFlakGameEnvironment::testSpec()
{
    // Configuration (dummy)
    HostConfiguration config;

    // Specification (dummy)
    game::spec::BeamVector_t beams;
    game::spec::Beam* b3 = beams.create(3);
    b3->setKillPower(333);
    b3->setDamagePower(777);

    game::spec::Beam* b4 = beams.create(4);
    b4->setKillPower(44);
    b4->setDamagePower(55);
    
    game::spec::TorpedoVector_t torps;
    game::spec::TorpedoLauncher* tl2 = torps.create(2);
    tl2->setKillPower(22);
    tl2->setDamagePower(123);

    // Testee
    game::vcr::flak::GameEnvironment testee(config, beams, torps);

    // Verify
    // - valid indexes
    TS_ASSERT_EQUALS(testee.getBeamKillPower(3), 333);
    TS_ASSERT_EQUALS(testee.getBeamDamagePower(3), 777);
    TS_ASSERT_EQUALS(testee.getBeamKillPower(4), 44);
    TS_ASSERT_EQUALS(testee.getBeamDamagePower(4), 55);
    TS_ASSERT_EQUALS(testee.getTorpedoKillPower(2), 22);
    TS_ASSERT_EQUALS(testee.getTorpedoDamagePower(2), 123);
    // - out-of-range
    TS_ASSERT_EQUALS(testee.getBeamKillPower(0), 0);
    TS_ASSERT_EQUALS(testee.getBeamDamagePower(0), 0);
    TS_ASSERT_EQUALS(testee.getTorpedoKillPower(0), 0);
    TS_ASSERT_EQUALS(testee.getTorpedoDamagePower(0), 0);
}
