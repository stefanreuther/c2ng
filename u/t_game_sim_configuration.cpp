/**
  *  \file u/t_game_sim_configuration.cpp
  *  \brief Test for game::sim::Configuration
  */

#include "game/sim/configuration.hpp"

#include "t_game_sim.hpp"
#include "afl/string/nulltranslator.hpp"

/** Setter/getter test. */
void
TestGameSimConfiguration::testIt()
{
    game::sim::Configuration t;

    // Initial state
    TS_ASSERT_EQUALS(t.getEngineShieldBonus(), 0);
    TS_ASSERT(t.hasScottyBonus());
    TS_ASSERT(!t.hasRandomLeftRight());
    TS_ASSERT(t.hasHonorAlliances());
    TS_ASSERT(!t.hasOnlyOneSimulation());
    TS_ASSERT(!t.hasSeedControl());
    TS_ASSERT(!t.hasRandomizeFCodesOnEveryFight());
    TS_ASSERT_EQUALS(t.getBalancingMode(), t.BalanceNone);
    TS_ASSERT_EQUALS(t.getMode(), t.VcrPHost4);
    TS_ASSERT(t.hasAlternativeCombat());

    // Accessors
    const game::sim::Configuration& ct = t;
    TS_ASSERT_EQUALS(&t.enemySettings(), &ct.enemySettings());
    TS_ASSERT_EQUALS(&t.allianceSettings(), &ct.allianceSettings());

    // Modify
    const game::config::HostConfiguration hostConfig;
    const game::TeamSettings teams;
    t.setMode(t.VcrHost, teams, hostConfig);
    TS_ASSERT_EQUALS(t.getMode(), t.VcrHost);
    TS_ASSERT(t.hasHonorAlliances());
    TS_ASSERT(!t.hasOnlyOneSimulation());
    TS_ASSERT(!t.hasSeedControl());
    TS_ASSERT(!t.hasRandomizeFCodesOnEveryFight());
    TS_ASSERT(!t.hasRandomLeftRight());
    TS_ASSERT_EQUALS(t.getBalancingMode(), t.Balance360k);
    TS_ASSERT(!t.hasAlternativeCombat());

    t.setEngineShieldBonus(10);
    TS_ASSERT_EQUALS(t.getEngineShieldBonus(), 10);

    t.setScottyBonus(false);
    TS_ASSERT(!t.hasScottyBonus());

    t.setRandomLeftRight(true);
    TS_ASSERT(t.hasRandomLeftRight());

    t.setHonorAlliances(false);
    TS_ASSERT(!t.hasHonorAlliances());

    t.setOnlyOneSimulation(true);
    TS_ASSERT(t.hasOnlyOneSimulation());

    t.setSeedControl(true);
    TS_ASSERT(t.hasSeedControl());

    t.setRandomizeFCodesOnEveryFight(true);
    TS_ASSERT(t.hasRandomizeFCodesOnEveryFight());

    t.setBalancingMode(t.BalanceMasterAtArms);
    TS_ASSERT_EQUALS(t.getBalancingMode(), t.BalanceMasterAtArms);

    // Cross interactions
    t.setOnlyOneSimulation(false);
    TS_ASSERT(!t.hasOnlyOneSimulation());
    TS_ASSERT(!t.hasSeedControl());

    t.setSeedControl(true);
    TS_ASSERT(t.hasOnlyOneSimulation());
    TS_ASSERT(t.hasSeedControl());

    // Load defaults
    t.loadDefaults(teams);
    TS_ASSERT(t.hasHonorAlliances());
    TS_ASSERT(!t.hasOnlyOneSimulation());
    TS_ASSERT(!t.hasSeedControl());
    TS_ASSERT(!t.hasRandomizeFCodesOnEveryFight());
}

/** Test configuration interaction. */
void
TestGameSimConfiguration::testConfig()
{
    const game::TeamSettings emptyTeams;
    {
        game::sim::Configuration t;
        game::config::HostConfiguration config;
        config[config.AllowEngineShieldBonus].set(true);
        config[config.EngineShieldBonusRate].set(30);
        config[config.AllowFedCombatBonus].set(true);
        config[config.NumExperienceLevels].set(3);
        t.setMode(t.VcrPHost4, emptyTeams, config);

        TS_ASSERT_EQUALS(t.getEngineShieldBonus(), 30);
        TS_ASSERT_EQUALS(t.hasScottyBonus(), true);
        TS_ASSERT_EQUALS(t.hasRandomLeftRight(), true);
        TS_ASSERT_EQUALS(t.getBalancingMode(), t.BalanceNone);
        TS_ASSERT_EQUALS(t.isExperienceEnabled(config), true);
    }
    {
        game::sim::Configuration t;
        game::config::HostConfiguration config;
        config[config.AllowEngineShieldBonus].set(false);
        config[config.EngineShieldBonusRate].set(30);
        config[config.AllowFedCombatBonus].set(true);
        config[config.NumExperienceLevels].set(0);
        t.setMode(t.VcrPHost4, emptyTeams, config);

        TS_ASSERT_EQUALS(t.getEngineShieldBonus(), 0);
        TS_ASSERT_EQUALS(t.hasScottyBonus(), true);
        TS_ASSERT_EQUALS(t.hasRandomLeftRight(), true);
        TS_ASSERT_EQUALS(t.getBalancingMode(), t.BalanceNone);
        TS_ASSERT_EQUALS(t.isExperienceEnabled(config), false);
    }
    {
        game::sim::Configuration t;
        game::config::HostConfiguration config;
        config[config.AllowEngineShieldBonus].set(true);
        config[config.EngineShieldBonusRate].set(30);
        config[config.AllowFedCombatBonus].set(true);
        config[config.NumExperienceLevels].set(3);
        t.setMode(t.VcrHost, emptyTeams, config);

        TS_ASSERT_EQUALS(t.getEngineShieldBonus(), 30);
        TS_ASSERT_EQUALS(t.hasScottyBonus(), true);
        TS_ASSERT_EQUALS(t.hasRandomLeftRight(), false);
        TS_ASSERT_EQUALS(t.getBalancingMode(), t.Balance360k);
        TS_ASSERT_EQUALS(t.isExperienceEnabled(config), false);
    }
    {
        game::sim::Configuration t;
        game::config::HostConfiguration config;
        config[config.AllowEngineShieldBonus].set(false);
        config[config.EngineShieldBonusRate].set(30);
        config[config.AllowFedCombatBonus].set(false);
        config[config.NumExperienceLevels].set(3);
        t.setMode(t.VcrHost, emptyTeams, config);

        TS_ASSERT_EQUALS(t.getEngineShieldBonus(), 00);
        TS_ASSERT_EQUALS(t.hasScottyBonus(), false);
        TS_ASSERT_EQUALS(t.hasRandomLeftRight(), false);
        TS_ASSERT_EQUALS(t.getBalancingMode(), t.Balance360k);
        TS_ASSERT_EQUALS(t.isExperienceEnabled(config), false);
    }
}

/** Test toString(). */
void
TestGameSimConfiguration::testToString()
{
    using game::sim::Configuration;
    afl::string::NullTranslator tx;
    TS_ASSERT(!toString(Configuration::VcrHost, tx).empty());
    TS_ASSERT(!toString(Configuration::VcrPHost2, tx).empty());
    TS_ASSERT(!toString(Configuration::VcrPHost3, tx).empty());
    TS_ASSERT(!toString(Configuration::VcrPHost4, tx).empty());
    TS_ASSERT(!toString(Configuration::VcrNuHost, tx).empty());
    TS_ASSERT(!toString(Configuration::VcrFLAK, tx).empty());

    TS_ASSERT(!toString(Configuration::BalanceNone, tx).empty());
    TS_ASSERT(!toString(Configuration::Balance360k, tx).empty());
    TS_ASSERT(!toString(Configuration::BalanceMasterAtArms, tx).empty());
}

/** Test copyFrom(). */
void
TestGameSimConfiguration::testCopyFrom()
{
    using game::sim::Configuration;

    Configuration orig;
    orig.setEngineShieldBonus(77);
    orig.allianceSettings().set(4, 5, true);
    orig.enemySettings().set(8, 2, true);

    Configuration copyAll;
    copyAll = orig;
    TS_ASSERT_EQUALS(copyAll.getEngineShieldBonus(), 77);
    TS_ASSERT_EQUALS(copyAll.allianceSettings().get(4, 5), true);
    TS_ASSERT_EQUALS(copyAll.enemySettings().get(8, 2), true);

    Configuration copyMain;
    copyMain.copyFrom(orig, Configuration::Areas_t(Configuration::MainArea));
    TS_ASSERT_EQUALS(copyMain.getEngineShieldBonus(), 77);
    TS_ASSERT_EQUALS(copyMain.allianceSettings().get(4, 5), false);
    TS_ASSERT_EQUALS(copyMain.enemySettings().get(8, 2), false);

    Configuration copyAlliance;
    copyAlliance.copyFrom(orig, Configuration::Areas_t(Configuration::AllianceArea));
    TS_ASSERT_EQUALS(copyAlliance.getEngineShieldBonus(), 0);
    TS_ASSERT_EQUALS(copyAlliance.allianceSettings().get(4, 5), true);
    TS_ASSERT_EQUALS(copyAlliance.enemySettings().get(8, 2), false);

    Configuration copyEnemy;
    copyEnemy.copyFrom(orig, Configuration::Areas_t(Configuration::EnemyArea));
    TS_ASSERT_EQUALS(copyEnemy.getEngineShieldBonus(), 0);
    TS_ASSERT_EQUALS(copyEnemy.allianceSettings().get(4, 5), false);
    TS_ASSERT_EQUALS(copyEnemy.enemySettings().get(8, 2), true);
}

/** Test getNext(). */
void
TestGameSimConfiguration::testGetNext()
{
    using game::sim::Configuration;

    // BalancingMode
    {
        Configuration::BalancingMode mode = Configuration::BalanceNone;
        int n = 0;
        do {
            ++n;
            mode = getNext(mode);
            TS_ASSERT(n < 100);
        } while (mode != Configuration::BalanceNone);
    }

    // VcrMode
    {
        Configuration::VcrMode mode = Configuration::VcrPHost4;
        int n = 0;
        do {
            ++n;
            mode = getNext(mode);
            TS_ASSERT(n < 100);
        } while (mode != Configuration::VcrPHost4);
    }
}
