/**
  *  \file test/game/config/hostconfigurationtest.cpp
  *  \brief Test for game::config::HostConfiguration
  */

#include "game/config/hostconfiguration.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/aliasoption.hpp"
#include "game/limits.hpp"

using game::config::HostConfiguration;

/** Test race number accesses. */
AFL_TEST("game.config.HostConfiguration:getPlayerRaceNumber", a)
{
    HostConfiguration testee;

    a.checkEqual("01", testee.getPlayerRaceNumber(1), 1);
    a.checkEqual("02", testee.getPlayerRaceNumber(5), 5);
    a.checkEqual("03", testee.getPlayerRaceNumber(20), 20);
    a.checkEqual("04", testee.getPlayerRaceNumber(1000), 1000);

    a.checkEqual("11", testee.getPlayerMissionNumber(1), 1);
    a.checkEqual("12", testee.getPlayerMissionNumber(5), 5);
    a.checkEqual("13", testee.getPlayerMissionNumber(20), 20);
    a.checkEqual("14", testee.getPlayerMissionNumber(1000), 1000);

    testee[HostConfiguration::PlayerRace].set(5, 3);
    testee[HostConfiguration::PlayerSpecialMission].set(1, 7);
    a.checkEqual("21", testee.getPlayerRaceNumber(1), 1);
    a.checkEqual("22", testee.getPlayerRaceNumber(5), 3);
    a.checkEqual("23", testee.getPlayerRaceNumber(20), 20);
    a.checkEqual("24", testee.getPlayerRaceNumber(1000), 1000);

    a.checkEqual("31", testee.getPlayerMissionNumber(1), 7);
    a.checkEqual("32", testee.getPlayerMissionNumber(5), 5);
    a.checkEqual("33", testee.getPlayerMissionNumber(20), 20);
    a.checkEqual("34", testee.getPlayerMissionNumber(1000), 1000);
}

/** Test configuration of aliases. */
AFL_TEST("game.config.HostConfiguration:aliases", a)
{
    HostConfiguration testee;

    // Get enumerator
    afl::base::Ref<HostConfiguration::Enumerator_t> e = testee.getOptions();

    // Count and verify options
    int numOptions = 0;
    int numAliases = 0;
    HostConfiguration::OptionInfo_t info;
    while (e->getNextElement(info)) {
        // Verify base properties
        a.checkNonNull("01. option pointer", info.second);
        a.check("02. option name", !info.first.empty());

        if (const game::config::AliasOption* opt = dynamic_cast<const game::config::AliasOption*>(info.second)) {
            // It's an alias option. Verify that it's valid.
            a.checkNonNull("11. getForwardedOption", opt->getForwardedOption());
            ++numAliases;
        } else {
            // It's a regular option.
            a.checkEqual("12. getSource", info.second->getSource(), game::config::ConfigurationOption::Default);
            ++numOptions;
        }
    }

    // Must have >5 aliases, >100 options (otherwise, our test logic is b0rked)
    a.check("21. num aliases", numAliases >= 5);
    a.check("22. num options", numOptions >= 100);
}

/** Test setDependantOptions(), "unset" case.
    SensorRange propagates to DarkSenseRange. */
AFL_TEST("game.config.HostConfiguration:setDependantOptions:unset", a)
{
    HostConfiguration testee;

    testee.setOption("sensorrange", "125", game::config::ConfigurationOption::Game);
    testee.setDependantOptions();

    a.checkEqual("01. SensorRange",    testee[HostConfiguration::SensorRange](1), 125);
    a.checkEqual("02. DarkSenseRange", testee[HostConfiguration::DarkSenseRange](1), 125);
}

/** Test setDependantOptions(), "set" case.
    SensorRange does not propagate to DarkSenseRange if set previously. */
AFL_TEST("game.config.HostConfiguration:setDependantOptions:set", a)
{
    HostConfiguration testee;

    testee.setOption("darksenserange", "204", game::config::ConfigurationOption::Game);
    testee.setOption("sensorrange", "125", game::config::ConfigurationOption::Game);
    testee.setDependantOptions();

    a.checkEqual("01. SensorRange",    testee[HostConfiguration::SensorRange](1), 125);
    a.checkEqual("02. DarkSenseRange", testee[HostConfiguration::DarkSenseRange](1), 204);
}

/** Test getExperienceLevelName(). */
AFL_TEST("game.config.HostConfiguration:getExperienceLevelName", a)
{
    afl::string::NullTranslator tx;
    HostConfiguration testee;

    testee.setOption("experiencelevelnames", "Erdwurm,Flugwapps, Ladehugo ,Nieswurz,Brotfahrer", game::config::ConfigurationOption::Game);

    a.checkEqual("01", testee.getExperienceLevelName(0, tx), "Erdwurm");
    a.checkEqual("02", testee.getExperienceLevelName(2, tx), "Ladehugo");
    a.checkEqual("03", testee.getExperienceLevelName(4, tx), "Brotfahrer");
    a.checkEqual("04", testee.getExperienceLevelName(5, tx), "Level 5");
}

/** Get getExperienceBonus(). */
AFL_TEST("game.config.HostConfiguration:getExperienceBonus", a)
{
    HostConfiguration testee;

    testee.setOption("emodbayrechargerate", "1,5,8,3", game::config::ConfigurationOption::Game);

    a.checkEqual("01", testee.getExperienceBonus(HostConfiguration::EModBayRechargeRate, 0), 0);
    a.checkEqual("02", testee.getExperienceBonus(HostConfiguration::EModBayRechargeRate, 1), 1);
    a.checkEqual("03", testee.getExperienceBonus(HostConfiguration::EModBayRechargeRate, 2), 5);
    a.checkEqual("04", testee.getExperienceBonus(HostConfiguration::EModBayRechargeRate, 4), 3);
    a.checkEqual("05", testee.getExperienceBonus(HostConfiguration::EModBayRechargeRate, 5), 3);  // option filled up
    a.checkEqual("06", testee.getExperienceBonus(HostConfiguration::EModBayRechargeRate, game::MAX_EXPERIENCE_LEVELS), 3);  // option filled up
    a.checkEqual("07", testee.getExperienceBonus(HostConfiguration::EModBayRechargeRate, 11), 0);  // out of range
}

/** Test getExperienceLevelFromPoints(). */
// Experience disabled
AFL_TEST("game.config.HostConfiguration:getExperienceLevelFromPoints:disabled", a)
{
    HostConfiguration testee;
    testee.setOption("NumExperienceLevels", "0", game::config::ConfigurationOption::Game);

    a.checkEqual("01", testee.getExperienceLevelFromPoints(0), 0);
    a.checkEqual("02", testee.getExperienceLevelFromPoints(5000), 0);
}

// Experience enabled
AFL_TEST("game.config.HostConfiguration:getExperienceLevelFromPoints:enabled", a)
{
    HostConfiguration testee;
    testee.setOption("NumExperienceLevels", "4", game::config::ConfigurationOption::Game);
    testee.setOption("ExperienceLevels", "750,1500,3000,4500,7000", game::config::ConfigurationOption::Game);

    a.checkEqual("01", testee.getExperienceLevelFromPoints(0), 0);
    a.checkEqual("02", testee.getExperienceLevelFromPoints(100), 0);
    a.checkEqual("03", testee.getExperienceLevelFromPoints(750), 1);
    a.checkEqual("04", testee.getExperienceLevelFromPoints(1499), 1);
    a.checkEqual("05", testee.getExperienceLevelFromPoints(1500), 2);
    a.checkEqual("06", testee.getExperienceLevelFromPoints(4500), 4);
    a.checkEqual("07", testee.getExperienceLevelFromPoints(8000), 4);
}

/*
 *  hasExtraFuelConsumption
 */

// Disabled
AFL_TEST("game.config.HostConfiguration:hasExtraFuelConsumption:off", a)
{
    HostConfiguration testee;
    testee.setOption("FuelUsagePerFightFor100KT", "0", game::config::ConfigurationOption::Game);
    testee.setOption("FuelUsagePerTurnFor100KT", "0", game::config::ConfigurationOption::Game);
    a.checkEqual("", testee.hasExtraFuelConsumption(), false);
}

// Partially enabled
AFL_TEST("game.config.HostConfiguration:hasExtraFuelConsumption:part", a)
{
    HostConfiguration testee;
    testee.setOption("FuelUsagePerFightFor100KT", "0", game::config::ConfigurationOption::Game);
    testee.setOption("FuelUsagePerTurnFor100KT", "0,0,0,0,0,1,0,0", game::config::ConfigurationOption::Game);
    a.checkEqual("", testee.hasExtraFuelConsumption(), true);
}

// Fully enabled
AFL_TEST("game.config.HostConfiguration:hasExtraFuelConsumption:on", a)
{
    HostConfiguration testee;
    testee.setOption("FuelUsagePerFightFor100KT", "5", game::config::ConfigurationOption::Game);
    testee.setOption("FuelUsagePerTurnFor100KT", "3", game::config::ConfigurationOption::Game);
    a.checkEqual("", testee.hasExtraFuelConsumption(), true);
}

/*
 *  hasDoubleTorpedoPower
 */

AFL_TEST("game.config.HostConfiguration:hasDoubleTorpedoPower:on", a)
{
    HostConfiguration testee;
    testee.setOption("AllowAlternativeCombat", "No", game::config::ConfigurationOption::Game);
    a.checkEqual("", testee.hasDoubleTorpedoPower(), true);
}

AFL_TEST("game.config.HostConfiguration:hasDoubleTorpedoPower:off", a)
{
    HostConfiguration testee;
    testee.setOption("AllowAlternativeCombat", "Yes", game::config::ConfigurationOption::Game);
    a.checkEqual("", testee.hasDoubleTorpedoPower(), false);
}
