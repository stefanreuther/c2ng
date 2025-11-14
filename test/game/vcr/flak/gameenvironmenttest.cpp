/**
  *  \file test/game/vcr/flak/gameenvironmenttest.cpp
  *  \brief Test for game::vcr::flak::GameEnvironment
  */

#include "game/vcr/flak/gameenvironment.hpp"

#include "afl/base/countof.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/torpedolauncher.hpp"

using afl::base::Ref;
using game::config::HostConfiguration;
using game::vcr::flak::Environment;

AFL_TEST("game.vcr.flak.GameEnvironment:config", a)
{
    // Configuration
    Ref<HostConfiguration> config = HostConfiguration::create();
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
        config->setOption(OPTIONS[i][0], OPTIONS[i][1], game::config::ConfigurationOption::Game);
    }

    // Specification (dummy)
    game::spec::BeamVector_t beams;
    game::spec::TorpedoVector_t torps;

    // Testee
    game::vcr::flak::GameEnvironment testee(*config, beams, torps);

    // Verify
    // - scalars
    a.checkEqual("01. AllowAlternativeCombat", testee.getConfiguration(Environment::AllowAlternativeCombat), 1);
    a.checkEqual("02. StandoffDistance", testee.getConfiguration(Environment::StandoffDistance), 32000);
    // - BayLaunchInterval array
    a.checkEqual("03. BayLaunchInterval", testee.getConfiguration(Environment::BayLaunchInterval, 0), 40);
    a.checkEqual("04. BayLaunchInterval", testee.getConfiguration(Environment::BayLaunchInterval, 1), 40);
    a.checkEqual("05. BayLaunchInterval", testee.getConfiguration(Environment::BayLaunchInterval, 10), 40);
    // - FighterKillOdds array
    a.checkEqual("06. FighterKillOdds", testee.getConfiguration(Environment::FighterKillOdds, 1), 80);
    a.checkEqual("07. FighterKillOdds", testee.getConfiguration(Environment::FighterKillOdds, 2), 90);
    a.checkEqual("08. FighterKillOdds", testee.getConfiguration(Environment::FighterKillOdds, 10), 70);
    // - BayRechargeBonus experience
    a.checkEqual("09. BayRechargeBonus", testee.getExperienceConfiguration(Environment::BayRechargeBonus, 0, 1), 3);
    a.checkEqual("10. BayRechargeBonus", testee.getExperienceConfiguration(Environment::BayRechargeBonus, 1, 1), 4);
    a.checkEqual("11. BayRechargeBonus", testee.getExperienceConfiguration(Environment::BayRechargeBonus, 2, 1), 5);
    a.checkEqual("12. BayRechargeBonus", testee.getExperienceConfiguration(Environment::BayRechargeBonus, 2, 10), 5);
    // - BeamHitFighterCharge experience
    a.checkEqual("13. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 0, 1), 900);
    a.checkEqual("14. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 1, 1), 870);
    a.checkEqual("15. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 2, 1), 830);
    a.checkEqual("16. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 3, 1), 810);
    a.checkEqual("17. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 4, 1), 750);
    a.checkEqual("18. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 0, 2), 800);
    a.checkEqual("19. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 1, 2), 770);
    a.checkEqual("20. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 2, 2), 730);
    a.checkEqual("21. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 3, 2), 710);
    a.checkEqual("22. BeamHitFighterCharge", testee.getExperienceConfiguration(Environment::BeamHitFighterCharge, 4, 2), 650);
    // - PlayerRace
    a.checkEqual("23. getPlayerRaceNumber", testee.getPlayerRaceNumber(0), 0);
    a.checkEqual("24. getPlayerRaceNumber", testee.getPlayerRaceNumber(1), 1);
    a.checkEqual("25. getPlayerRaceNumber", testee.getPlayerRaceNumber(2), 1);
    a.checkEqual("26. getPlayerRaceNumber", testee.getPlayerRaceNumber(4), 4);
    a.checkEqual("27. getPlayerRaceNumber", testee.getPlayerRaceNumber(game::MAX_PLAYERS), 5);
    a.checkEqual("28. getPlayerRaceNumber", testee.getPlayerRaceNumber(game::MAX_PLAYERS+1), game::MAX_PLAYERS+1);
    a.checkEqual("29. getPlayerRaceNumber", testee.getPlayerRaceNumber(100), 100);
}

AFL_TEST("game.vcr.flak.GameEnvironment:spec", a)
{
    // Configuration (dummy)
    Ref<HostConfiguration> config = HostConfiguration::create();

    // Specification
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
    game::vcr::flak::GameEnvironment testee(*config, beams, torps);

    // Verify
    // - valid indexes
    a.checkEqual("01. getBeamKillPower",      testee.getBeamKillPower(3), 333);
    a.checkEqual("02. getBeamDamagePower",    testee.getBeamDamagePower(3), 777);
    a.checkEqual("03. getBeamKillPower",      testee.getBeamKillPower(4), 44);
    a.checkEqual("04. getBeamDamagePower",    testee.getBeamDamagePower(4), 55);
    a.checkEqual("05. getTorpedoKillPower",   testee.getTorpedoKillPower(2), 22);
    a.checkEqual("06. getTorpedoDamagePower", testee.getTorpedoDamagePower(2), 123);
    // - out-of-range
    a.checkEqual("07. getBeamKillPower",      testee.getBeamKillPower(0), 0);
    a.checkEqual("08. getBeamDamagePower",    testee.getBeamDamagePower(0), 0);
    a.checkEqual("09. getTorpedoKillPower",   testee.getTorpedoKillPower(0), 0);
    a.checkEqual("10. getTorpedoDamagePower", testee.getTorpedoDamagePower(0), 0);
}
