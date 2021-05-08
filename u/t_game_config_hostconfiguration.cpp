/**
  *  \file u/t_game_config_hostconfiguration.cpp
  *  \brief Test for game::config::HostConfiguration
  */

#include "game/config/hostconfiguration.hpp"

#include "t_game_config.hpp"
#include "game/config/aliasoption.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/limits.hpp"

/** Test race number accesses. */
void
TestGameConfigHostConfiguration::testRace()
{
    game::config::HostConfiguration testee;

    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(1), 1);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(5), 5);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(20), 20);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(1000), 1000);

    TS_ASSERT_EQUALS(testee.getPlayerMissionNumber(1), 1);
    TS_ASSERT_EQUALS(testee.getPlayerMissionNumber(5), 5);
    TS_ASSERT_EQUALS(testee.getPlayerMissionNumber(20), 20);
    TS_ASSERT_EQUALS(testee.getPlayerMissionNumber(1000), 1000);

    testee[testee.PlayerRace].set(5, 3);
    testee[testee.PlayerSpecialMission].set(1, 7);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(1), 1);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(5), 3);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(20), 20);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(1000), 1000);

    TS_ASSERT_EQUALS(testee.getPlayerMissionNumber(1), 7);
    TS_ASSERT_EQUALS(testee.getPlayerMissionNumber(5), 5);
    TS_ASSERT_EQUALS(testee.getPlayerMissionNumber(20), 20);
    TS_ASSERT_EQUALS(testee.getPlayerMissionNumber(1000), 1000);
}

/** Test configuration of aliases. */
void
TestGameConfigHostConfiguration::testAlias()
{
    using game::config::HostConfiguration;
    HostConfiguration testee;

    // Get enumerator
    afl::base::Ref<HostConfiguration::Enumerator_t> e = testee.getOptions();

    // Count and verify options
    int numOptions = 0;
    int numAliases = 0;
    HostConfiguration::OptionInfo_t info;
    while (e->getNextElement(info)) {
        // Verify base properties
        TS_ASSERT(info.second != 0);
        TS_ASSERT(!info.first.empty());

        if (const game::config::AliasOption* opt = dynamic_cast<const game::config::AliasOption*>(info.second)) {
            // It's an alias option. Verify that it's valid.
            TS_ASSERT(opt->getForwardedOption() != 0);
            ++numAliases;
        } else {
            // It's a regular option.
            TS_ASSERT_EQUALS(info.second->getSource(), game::config::ConfigurationOption::Default);
            ++numOptions;
        }
    }

    // Must have >5 aliases, >100 options (otherwise, our test logic is b0rked)
    TS_ASSERT_LESS_THAN_EQUALS(5, numAliases);
    TS_ASSERT_LESS_THAN_EQUALS(100, numOptions);
}

/** Test setDependantOptions(), "unset" case.
    SensorRange propagates to DarkSenseRange. */
void
TestGameConfigHostConfiguration::testDependant1()
{
    using game::config::HostConfiguration;
    HostConfiguration testee;

    testee.setOption("sensorrange", "125", game::config::ConfigurationOption::Game);
    testee.setDependantOptions();

    TS_ASSERT_EQUALS(testee[HostConfiguration::SensorRange](1), 125);
    TS_ASSERT_EQUALS(testee[HostConfiguration::DarkSenseRange](1), 125);
}

/** Test setDependantOptions(), "set" case.
    SensorRange does not propagate to DarkSenseRange if set previously. */
void
TestGameConfigHostConfiguration::testDependant2()
{
    using game::config::HostConfiguration;
    HostConfiguration testee;

    testee.setOption("darksenserange", "204", game::config::ConfigurationOption::Game);
    testee.setOption("sensorrange", "125", game::config::ConfigurationOption::Game);
    testee.setDependantOptions();

    TS_ASSERT_EQUALS(testee[HostConfiguration::SensorRange](1), 125);
    TS_ASSERT_EQUALS(testee[HostConfiguration::DarkSenseRange](1), 204);
}

/** Test getExperienceLevelName(). */
void
TestGameConfigHostConfiguration::testExperienceName()
{
    afl::string::NullTranslator tx;
    game::config::HostConfiguration testee;

    testee.setOption("experiencelevelnames", "Erdwurm,Flugwapps, Ladehugo ,Nieswurz,Brotfahrer", game::config::ConfigurationOption::Game);

    TS_ASSERT_EQUALS(testee.getExperienceLevelName(0, tx), "Erdwurm");
    TS_ASSERT_EQUALS(testee.getExperienceLevelName(2, tx), "Ladehugo");
    TS_ASSERT_EQUALS(testee.getExperienceLevelName(4, tx), "Brotfahrer");
    TS_ASSERT_EQUALS(testee.getExperienceLevelName(5, tx), "Level 5");
}

/** Get getExperienceBonus(). */
void
TestGameConfigHostConfiguration::testExperienceBonus()
{
    game::config::HostConfiguration testee;

    testee.setOption("emodbayrechargerate", "1,5,8,3", game::config::ConfigurationOption::Game);

    TS_ASSERT_EQUALS(testee.getExperienceBonus(game::config::HostConfiguration::EModBayRechargeRate, 0), 0);
    TS_ASSERT_EQUALS(testee.getExperienceBonus(game::config::HostConfiguration::EModBayRechargeRate, 1), 1);
    TS_ASSERT_EQUALS(testee.getExperienceBonus(game::config::HostConfiguration::EModBayRechargeRate, 2), 5);
    TS_ASSERT_EQUALS(testee.getExperienceBonus(game::config::HostConfiguration::EModBayRechargeRate, 4), 3);
    TS_ASSERT_EQUALS(testee.getExperienceBonus(game::config::HostConfiguration::EModBayRechargeRate, 5), 3);  // option filled up
    TS_ASSERT_EQUALS(testee.getExperienceBonus(game::config::HostConfiguration::EModBayRechargeRate, game::MAX_EXPERIENCE_LEVELS), 3);  // option filled up
    TS_ASSERT_EQUALS(testee.getExperienceBonus(game::config::HostConfiguration::EModBayRechargeRate, 11), 0);  // out of range
}

/** Test getExperienceLevelFromPoints(). */
void
TestGameConfigHostConfiguration::testGetExperienceLevelFromPoints()
{
    // Experience disabled
    {
        game::config::HostConfiguration testee;
        testee.setOption("NumExperienceLevels", "0", game::config::ConfigurationOption::Game);

        TS_ASSERT_EQUALS(testee.getExperienceLevelFromPoints(0), 0);
        TS_ASSERT_EQUALS(testee.getExperienceLevelFromPoints(5000), 0);
    }

    // Experience enabled
    {
        game::config::HostConfiguration testee;
        testee.setOption("NumExperienceLevels", "4", game::config::ConfigurationOption::Game);
        testee.setOption("ExperienceLevels", "750,1500,3000,4500,7000", game::config::ConfigurationOption::Game);

        TS_ASSERT_EQUALS(testee.getExperienceLevelFromPoints(0), 0);
        TS_ASSERT_EQUALS(testee.getExperienceLevelFromPoints(100), 0);
        TS_ASSERT_EQUALS(testee.getExperienceLevelFromPoints(750), 1);
        TS_ASSERT_EQUALS(testee.getExperienceLevelFromPoints(1499), 1);
        TS_ASSERT_EQUALS(testee.getExperienceLevelFromPoints(1500), 2);
        TS_ASSERT_EQUALS(testee.getExperienceLevelFromPoints(4500), 4);
        TS_ASSERT_EQUALS(testee.getExperienceLevelFromPoints(8000), 4);
    }
}

