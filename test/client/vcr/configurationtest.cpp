/**
  *  \file test/client/vcr/configurationtest.cpp
  *  \brief Test for client::vcr::Configuration
  */

#include "client/vcr/configuration.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/test/root.hpp"
#include "afl/test/translator.hpp"

using client::vcr::Configuration;
using game::config::UserConfiguration;
using game::proxy::ConfigurationProxy;
using game::test::makeRoot;
using game::HostVersion;

/*
 *  Init and I/O
 */

/* Check initialisation. */
AFL_TEST("client.vcr.Configuration:init", a)
{
    // Create default
    Configuration testee;

    // Verify
    a.checkGreaterThan("getTickInterval",           testee.getTickInterval(), 0);
    a.checkGreaterThan("getNumTicksPerBattleCycle", testee.getNumTicksPerBattleCycle(), 0);

    a.checkEqual("getSpeed",            testee.getSpeed(), 2);
    a.checkEqual("getRendererMode",     testee.getRendererMode(),     UserConfiguration::StandardRenderer);
    a.checkEqual("getEffectsMode",      testee.getEffectsMode(),      UserConfiguration::StandardEffects);
    a.checkEqual("getFlakRendererMode", testee.getFlakRendererMode(), UserConfiguration::ThreeDMode);
    a.checkEqual("hasFlakGrid",         testee.hasFlakGrid(), true);
}

/* Check load from default configuration.
   Results must be the same as for default initialisation. */
AFL_TEST("client.vcr.Configuration:load:empty", a)
{
    // Create session
    game::test::SessionThread h;
    h.session().setRoot(makeRoot(HostVersion()).asPtr());
    game::test::WaitIndicator ind;
    ConfigurationProxy proxy(h.gameSender());

    // Load
    Configuration testee;
    testee.load(ind, proxy);

    // Verify
    a.checkGreaterThan("getTickInterval",           testee.getTickInterval(), 0);
    a.checkGreaterThan("getNumTicksPerBattleCycle", testee.getNumTicksPerBattleCycle(), 0);

    a.checkEqual("getSpeed",            testee.getSpeed(), 2);
    a.checkEqual("getRendererMode",     testee.getRendererMode(),     UserConfiguration::StandardRenderer);
    a.checkEqual("getEffectsMode",      testee.getEffectsMode(),      UserConfiguration::StandardEffects);
    a.checkEqual("getFlakRendererMode", testee.getFlakRendererMode(), UserConfiguration::ThreeDMode);
    a.checkEqual("hasFlakGrid",         testee.hasFlakGrid(), true);
}

/* Check load from valid configuration. */
AFL_TEST("client.vcr.Configuration:load:normal", a)
{
    // Create session
    game::test::SessionThread h;
    h.session().setRoot(makeRoot(HostVersion()).asPtr());
    game::test::WaitIndicator ind;
    ConfigurationProxy proxy(h.gameSender());

    // Modify
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config[UserConfiguration::Vcr_Speed].set(7);
    config[UserConfiguration::Vcr_Renderer].set(2);
    config[UserConfiguration::Vcr_Effects].set(1);
    config[UserConfiguration::Flak_Renderer].set(1);
    config[UserConfiguration::Flak_Grid].set(0);

    // Load
    Configuration testee;
    testee.load(ind, proxy);

    // Verify
    a.checkGreaterThan("getTickInterval",           testee.getTickInterval(), 0);
    a.checkGreaterThan("getNumTicksPerBattleCycle", testee.getNumTicksPerBattleCycle(), 0);

    a.checkEqual("getSpeed",            testee.getSpeed(), 7);
    a.checkEqual("getRendererMode",     testee.getRendererMode(),     UserConfiguration::InterleavedRenderer);
    a.checkEqual("getEffectsMode",      testee.getEffectsMode(),      UserConfiguration::SimpleEffects);
    a.checkEqual("getFlakRendererMode", testee.getFlakRendererMode(), UserConfiguration::FlatMode);
    a.checkEqual("hasFlakGrid",         testee.hasFlakGrid(), false);
}

/* Check load from invalid configuration */
AFL_TEST("client.vcr.Configuration:load:out-of-range", a)
{
    // Create session
    game::test::SessionThread h;
    h.session().setRoot(makeRoot(HostVersion()).asPtr());
    game::test::WaitIndicator ind;
    ConfigurationProxy proxy(h.gameSender());

    // Modify
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config[UserConfiguration::Vcr_Speed].set(777);
    config[UserConfiguration::Vcr_Renderer].set(222);
    config[UserConfiguration::Vcr_Effects].set(111);
    config[UserConfiguration::Flak_Renderer].set(111);
    config[UserConfiguration::Flak_Grid].set(999);

    // Load
    Configuration testee;
    testee.load(ind, proxy);

    // Verify
    a.checkGreaterThan("getTickInterval",           testee.getTickInterval(), 0);
    a.checkGreaterThan("getNumTicksPerBattleCycle", testee.getNumTicksPerBattleCycle(), 0);

    a.checkEqual("getSpeed",            testee.getSpeed(), Configuration::SLOWEST_SPEED);
    a.checkEqual("getRendererMode",     testee.getRendererMode(),     UserConfiguration::StandardRenderer);
    a.checkEqual("getEffectsMode",      testee.getEffectsMode(),      UserConfiguration::StandardEffects);
    a.checkEqual("getFlakRendererMode", testee.getFlakRendererMode(), UserConfiguration::ThreeDMode);
    a.checkEqual("hasFlakGrid",         testee.hasFlakGrid(), true);
}

/* Check save. */
AFL_TEST("client.vcr.Configuration:save", a)
{
    // Create session
    game::test::SessionThread h;
    h.session().setRoot(makeRoot(HostVersion()).asPtr());
    game::test::WaitIndicator ind;
    ConfigurationProxy proxy(h.gameSender());

    // Configure
    Configuration testee;
    testee.setSpeed(7);
    testee.setRendererMode(UserConfiguration::InterleavedRenderer);
    testee.setEffectsMode(UserConfiguration::SimpleEffects);
    testee.setFlakRendererMode(UserConfiguration::FlatMode);
    testee.setFlakGrid(false);

    // Save
    testee.save(proxy);
    h.sync();

    // Verify
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    a.checkEqual("Vcr_Speed",     config[UserConfiguration::Vcr_Speed](), 7);
    a.checkEqual("Vcr_Renderer",  config[UserConfiguration::Vcr_Renderer](), 2);
    a.checkEqual("Vcr_Effects",   config[UserConfiguration::Vcr_Effects](), 1);
    a.checkEqual("Flak_Renderer", config[UserConfiguration::Flak_Renderer](), 1);
    a.checkEqual("Flak_Grid",     config[UserConfiguration::Flak_Grid](), 0);
}

/*
 *  Individual properties
 */

/* Speed changes and mapping of speed to tick parameters */
AFL_TEST("client.vcr.Configuration:changeSpeed", a)
{
    Configuration testee;
    testee.setSpeed(7);
    int originalInterval = testee.getNumTicksPerBattleCycle() * testee.getTickInterval();
    a.checkEqual("getSpeed 1", testee.getSpeed(), 7);
    a.checkGreaterThan("interval 1", originalInterval, 0);

    testee.changeSpeed(1);
    int slowerInterval = testee.getNumTicksPerBattleCycle() * testee.getTickInterval();
    a.checkEqual("getSpeed 2", testee.getSpeed(), 8);
    a.checkGreaterThan("interval 2", slowerInterval, originalInterval);

    testee.changeSpeed(-5);
    a.checkEqual("getSpeed 3", testee.getSpeed(), 3);

    testee.changeSpeed(-5);
    int fastestInterval = testee.getNumTicksPerBattleCycle() * testee.getTickInterval();
    a.checkEqual("getSpeed 4", testee.getSpeed(), Configuration::FASTEST_SPEED);
    a.checkGreaterThan("interval 4", originalInterval, fastestInterval);
}

/* Cycling of renderer modes */
AFL_TEST("client.vcr.Configuration:cycleRendererMode", a)
{
    Configuration testee;

    UserConfiguration::RendererMode m = testee.getRendererMode();
    int i = 0;
    do {
        testee.cycleRendererMode();
        ++i;
        a.checkLessThan("loops", i, 100);
    } while (testee.getRendererMode() != m);
}

/* Cycling of effect modes */
AFL_TEST("client.vcr.Configuration:cycleEffectsMode", a)
{
    Configuration testee;

    UserConfiguration::EffectsMode m = testee.getEffectsMode();
    int i = 0;
    do {
        testee.cycleEffectsMode();
        ++i;
        a.checkLessThan("loops", i, 100);
    } while (testee.getEffectsMode() != m);
}

/* Cycling of FLAK renderer modes */
AFL_TEST("client.vcr.Configuration:cycleFlakRendererMode", a)
{
    Configuration testee;

    UserConfiguration::FlakRendererMode m = testee.getFlakRendererMode();
    int i = 0;
    do {
        testee.cycleFlakRendererMode();
        ++i;
        a.checkLessThan("loops", i, 100);
    } while (testee.getFlakRendererMode() != m);
}

/* Toggling of FLAK renderer modes */
AFL_TEST("client.vcr.Configuration:toggleFlakRendererMode", a)
{
    Configuration testee;
    testee.setFlakRendererMode(UserConfiguration::ThreeDMode);

    testee.toggleFlakRendererMode(UserConfiguration::ThreeDMode, UserConfiguration::FlatMode);
    a.checkEqual("toggle away", testee.getFlakRendererMode(), UserConfiguration::FlatMode);

    testee.toggleFlakRendererMode(UserConfiguration::ThreeDMode, UserConfiguration::FlatMode);
    a.checkEqual("toggle towards", testee.getFlakRendererMode(), UserConfiguration::ThreeDMode);
}

/* Toggling grid */
AFL_TEST("client.vcr.Configuration:toggleFlakGrid", a)
{
    Configuration testee;
    testee.setFlakGrid(true);

    testee.toggleFlakGrid();
    a.checkEqual("toggle off", testee.hasFlakGrid(), false);

    testee.toggleFlakGrid();
    a.checkEqual("toggle on", testee.hasFlakGrid(), true);
}

/* Naming speeds */
AFL_TEST("client.vcr.Configuration:getSpeedName", a)
{
    afl::test::Translator tx("<", ">");
    a.checkEqual("fastest", Configuration::getSpeedName(Configuration::FASTEST_SPEED, tx), "<fastest>");
    a.checkEqual("slowest", Configuration::getSpeedName(Configuration::SLOWEST_SPEED, tx), "<slowest>");

    a.checkDifferent("3", Configuration::getSpeedName(3, tx), "");
}
