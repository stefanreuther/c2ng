/**
  *  \file u/t_game_sim_configuration.cpp
  *  \brief Test for game::sim::Configuration
  */

#include "game/sim/configuration.hpp"

#include "t_game_sim.hpp"

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

