/**
  *  \file test/game/sim/configurationtest.cpp
  *  \brief Test for game::sim::Configuration
  */

#include "game/sim/configuration.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

using game::HostVersion;
using game::sim::Configuration;
using game::config::HostConfiguration;

/** Setter/getter test. */
AFL_TEST("game.sim.Configuration:basics", a)
{
    Configuration t;

    // Initial state
    a.checkEqual("01. getEngineShieldBonus", t.getEngineShieldBonus(), 0);
    a.check("02. hasScottyBonus", t.hasScottyBonus());
    a.check("03. hasRandomLeftRight", !t.hasRandomLeftRight());
    a.check("04. hasHonorAlliances", t.hasHonorAlliances());
    a.check("05. hasOnlyOneSimulation", !t.hasOnlyOneSimulation());
    a.check("06. hasSeedControl", !t.hasSeedControl());
    a.check("07. hasRandomizeFCodesOnEveryFight", !t.hasRandomizeFCodesOnEveryFight());
    a.checkEqual("08. getBalancingMode", t.getBalancingMode(), t.BalanceNone);
    a.checkEqual("09. getMode", t.getMode(), t.VcrPHost4);
    a.check("10. hasAlternativeCombat", t.hasAlternativeCombat());

    // Accessors
    const Configuration& ct = t;
    a.checkEqual("11. enemySettings", &t.enemySettings(), &ct.enemySettings());
    a.checkEqual("12. allianceSettings", &t.allianceSettings(), &ct.allianceSettings());

    // Modify
    const HostConfiguration hostConfig;
    t.setMode(t.VcrHost, 0, hostConfig);
    a.checkEqual("21. getMode", t.getMode(), t.VcrHost);
    a.check("22. hasHonorAlliances", t.hasHonorAlliances());
    a.check("23. hasOnlyOneSimulation", !t.hasOnlyOneSimulation());
    a.check("24. hasSeedControl", !t.hasSeedControl());
    a.check("25. hasRandomizeFCodesOnEveryFight", !t.hasRandomizeFCodesOnEveryFight());
    a.check("26. hasRandomLeftRight", !t.hasRandomLeftRight());
    a.checkEqual("27. getBalancingMode", t.getBalancingMode(), t.Balance360k);
    a.check("28. hasAlternativeCombat", !t.hasAlternativeCombat());

    t.setEngineShieldBonus(10);
    a.checkEqual("31. getEngineShieldBonus", t.getEngineShieldBonus(), 10);

    t.setScottyBonus(false);
    a.check("41. hasScottyBonus", !t.hasScottyBonus());

    t.setRandomLeftRight(true);
    a.check("51. hasRandomLeftRight", t.hasRandomLeftRight());

    t.setHonorAlliances(false);
    a.check("61. hasHonorAlliances", !t.hasHonorAlliances());

    t.setOnlyOneSimulation(true);
    a.check("71. hasOnlyOneSimulation", t.hasOnlyOneSimulation());

    t.setSeedControl(true);
    a.check("81. hasSeedControl", t.hasSeedControl());

    t.setRandomizeFCodesOnEveryFight(true);
    a.check("91. hasRandomizeFCodesOnEveryFight", t.hasRandomizeFCodesOnEveryFight());

    t.setBalancingMode(t.BalanceMasterAtArms);
    a.checkEqual("101. getBalancingMode", t.getBalancingMode(), t.BalanceMasterAtArms);

    // Cross interactions
    t.setOnlyOneSimulation(false);
    a.check("111. hasOnlyOneSimulation", !t.hasOnlyOneSimulation());
    a.check("112. hasSeedControl", !t.hasSeedControl());

    t.setSeedControl(true);
    a.check("121. hasOnlyOneSimulation", t.hasOnlyOneSimulation());
    a.check("122. hasSeedControl", t.hasSeedControl());

    // Load defaults
    t = Configuration();      // formerly, loadDefaults
    a.check("131. hasHonorAlliances", t.hasHonorAlliances());
    a.check("132. hasOnlyOneSimulation", !t.hasOnlyOneSimulation());
    a.check("133. hasSeedControl", !t.hasSeedControl());
    a.check("134. hasRandomizeFCodesOnEveryFight", !t.hasRandomizeFCodesOnEveryFight());
}

/** Test configuration interaction. */
AFL_TEST("game.sim.Configuration:config:phost:all-on", a)
{
    Configuration t;
    HostConfiguration config;
    config[config.AllowEngineShieldBonus].set(true);
    config[config.EngineShieldBonusRate].set(30);
    config[config.AllowFedCombatBonus].set(true);
    config[config.NumExperienceLevels].set(3);
    t.setMode(t.VcrPHost4, 0, config);

    a.checkEqual("01. getEngineShieldBonus", t.getEngineShieldBonus(), 30);
    a.checkEqual("02. hasScottyBonus",       t.hasScottyBonus(), true);
    a.checkEqual("03. hasRandomLeftRight",   t.hasRandomLeftRight(), true);
    a.checkEqual("04. getBalancingMode",     t.getBalancingMode(), t.BalanceNone);
    a.checkEqual("05. isExperienceEnabled",  t.isExperienceEnabled(config), true);
}

AFL_TEST("game.sim.Configuration:config:phost:no-esb", a)
{
    Configuration t;
    HostConfiguration config;
    config[config.AllowEngineShieldBonus].set(false);
    config[config.EngineShieldBonusRate].set(30);
    config[config.AllowFedCombatBonus].set(true);
    config[config.NumExperienceLevels].set(0);
    t.setMode(t.VcrPHost4, 0, config);

    a.checkEqual("11. getEngineShieldBonus", t.getEngineShieldBonus(), 0);
    a.checkEqual("12. hasScottyBonus",       t.hasScottyBonus(), true);
    a.checkEqual("13. hasRandomLeftRight",   t.hasRandomLeftRight(), true);
    a.checkEqual("14. getBalancingMode",     t.getBalancingMode(), t.BalanceNone);
    a.checkEqual("15. isExperienceEnabled",  t.isExperienceEnabled(config), false);
}

AFL_TEST("game.sim.Configuration:config:host:all-on", a)
{
    Configuration t;
    HostConfiguration config;
    config[config.AllowEngineShieldBonus].set(true);
    config[config.EngineShieldBonusRate].set(30);
    config[config.AllowFedCombatBonus].set(true);
    config[config.NumExperienceLevels].set(3);
    t.setMode(t.VcrHost, 0, config);

    a.checkEqual("21. getEngineShieldBonus", t.getEngineShieldBonus(), 30);
    a.checkEqual("22. hasScottyBonus",       t.hasScottyBonus(), true);
    a.checkEqual("23. hasRandomLeftRight",   t.hasRandomLeftRight(), false);
    a.checkEqual("24. getBalancingMode",     t.getBalancingMode(), t.Balance360k);
    a.checkEqual("25. isExperienceEnabled",  t.isExperienceEnabled(config), false);
}

AFL_TEST("game.sim.Configuration:config:host:all-off", a)
{
    Configuration t;
    HostConfiguration config;
    config[config.AllowEngineShieldBonus].set(false);
    config[config.EngineShieldBonusRate].set(30);
    config[config.AllowFedCombatBonus].set(false);
    config[config.NumExperienceLevels].set(3);
    t.setMode(t.VcrHost, 0, config);

    a.checkEqual("31. getEngineShieldBonus", t.getEngineShieldBonus(), 00);
    a.checkEqual("32. hasScottyBonus",       t.hasScottyBonus(), false);
    a.checkEqual("33. hasRandomLeftRight",   t.hasRandomLeftRight(), false);
    a.checkEqual("34. getBalancingMode",     t.getBalancingMode(), t.Balance360k);
    a.checkEqual("35. isExperienceEnabled",  t.isExperienceEnabled(config), false);
}


/** Test toString(). */
AFL_TEST("game.sim.Configuration:toString", a)
{
    afl::string::NullTranslator tx;
    a.check("01", !toString(Configuration::VcrHost, tx).empty());
    a.check("02", !toString(Configuration::VcrPHost2, tx).empty());
    a.check("03", !toString(Configuration::VcrPHost3, tx).empty());
    a.check("04", !toString(Configuration::VcrPHost4, tx).empty());
    a.check("05", !toString(Configuration::VcrNuHost, tx).empty());
    a.check("06", !toString(Configuration::VcrFLAK, tx).empty());

    a.check("11", !toString(Configuration::BalanceNone, tx).empty());
    a.check("12", !toString(Configuration::Balance360k, tx).empty());
    a.check("13", !toString(Configuration::BalanceMasterAtArms, tx).empty());
}

/** Test copyFrom(). */
AFL_TEST("game.sim.Configuration:copyFrom", a)
{
    Configuration orig;
    orig.setEngineShieldBonus(77);
    orig.allianceSettings().set(4, 5, true);
    orig.enemySettings().set(8, 2, true);

    Configuration copyAll;
    copyAll = orig;
    a.checkEqual("01. getEngineShieldBonus", copyAll.getEngineShieldBonus(), 77);
    a.checkEqual("02. allianceSettings",     copyAll.allianceSettings().get(4, 5), true);
    a.checkEqual("03. enemySettings",        copyAll.enemySettings().get(8, 2), true);

    Configuration copyMain;
    copyMain.copyFrom(orig, Configuration::Areas_t(Configuration::MainArea));
    a.checkEqual("11. getEngineShieldBonus", copyMain.getEngineShieldBonus(), 77);
    a.checkEqual("12. allianceSettings",     copyMain.allianceSettings().get(4, 5), false);
    a.checkEqual("13. enemySettings",        copyMain.enemySettings().get(8, 2), false);

    Configuration copyAlliance;
    copyAlliance.copyFrom(orig, Configuration::Areas_t(Configuration::AllianceArea));
    a.checkEqual("21. getEngineShieldBonus", copyAlliance.getEngineShieldBonus(), 0);
    a.checkEqual("22. allianceSettings",     copyAlliance.allianceSettings().get(4, 5), true);
    a.checkEqual("23. enemySettings",        copyAlliance.enemySettings().get(8, 2), false);

    Configuration copyEnemy;
    copyEnemy.copyFrom(orig, Configuration::Areas_t(Configuration::EnemyArea));
    a.checkEqual("31. getEngineShieldBonus", copyEnemy.getEngineShieldBonus(), 0);
    a.checkEqual("32. allianceSettings",     copyEnemy.allianceSettings().get(4, 5), false);
    a.checkEqual("33. enemySettings",        copyEnemy.enemySettings().get(8, 2), true);
}

/** Test getNext(). */

// BalancingMode
AFL_TEST("game.sim.Configuration:getNext:BalancingMode", a)
{
    Configuration::BalancingMode mode = Configuration::BalanceNone;
    int n = 0;
    do {
        ++n;
        mode = getNext(mode);
        a.check("01", n < 100);
    } while (mode != Configuration::BalanceNone);
}

// VcrMode
AFL_TEST("game.sim.Configuration:getNext:VcrMode", a)
{
    Configuration::VcrMode mode = Configuration::VcrPHost4;
    int n = 0;
    do {
        ++n;
        mode = getNext(mode);
        a.check("11", n < 100);
    } while (mode != Configuration::VcrPHost4);
}

/** Test setModeFromHostVersion(). */
AFL_TEST("game.sim.Configuration:setModeFromHostVersion:Host", a)
{
    HostConfiguration config;
    Configuration t;
    t.setModeFromHostVersion(HostVersion(HostVersion::Host, MKVERSION(3,22,0)), 0, config);
    a.checkEqual("getMode", t.getMode(), Configuration::VcrHost);
}

AFL_TEST("game.sim.Configuration:setModeFromHostVersion:NuHost", a)
{
    HostConfiguration config;
    Configuration t;
    t.setModeFromHostVersion(HostVersion(HostVersion::NuHost, MKVERSION(3,22,0)), 0, config);
    a.checkEqual("getMode", t.getMode(), Configuration::VcrNuHost);
}

AFL_TEST("game.sim.Configuration:setModeFromHostVersion:PHost:2", a)
{
    HostConfiguration config;
    Configuration t;
    t.setModeFromHostVersion(HostVersion(HostVersion::PHost, MKVERSION(2,0,1)), 0, config);
    a.checkEqual("getMode", t.getMode(), Configuration::VcrPHost2);
}

AFL_TEST("game.sim.Configuration:setModeFromHostVersion:PHost:3", a)
{
    HostConfiguration config;
    Configuration t;
    t.setModeFromHostVersion(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), 0, config);
    a.checkEqual("getMode", t.getMode(), Configuration::VcrPHost3);
}

AFL_TEST("game.sim.Configuration:setModeFromHostVersion:PHost:4", a)
{
    HostConfiguration config;
    Configuration t;
    t.setModeFromHostVersion(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)), 0, config);
    a.checkEqual("getMode", t.getMode(), Configuration::VcrPHost4);
}
