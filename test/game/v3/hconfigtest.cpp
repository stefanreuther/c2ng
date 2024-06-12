/**
  *  \file test/game/v3/hconfigtest.cpp
  *  \brief Test for game::v3::HConfig
  */

#include "game/v3/hconfig.hpp"

#include "afl/test/testrunner.hpp"
#include "game/v3/structures.hpp"

using game::v3::structures::HConfig;
using game::config::HostConfiguration;
using game::config::Configuration;

/** Test that packHConfig() initializes everything. */
AFL_TEST("game.v3.HConfig:pack", a)
{
    // Prepare
    HConfig fig;
    afl::base::Bytes_t bytes(afl::base::fromObject(fig));
    bytes.fill(0xE1);

    // Pack a default host configuration
    HostConfiguration config;
    game::v3::packHConfig(fig, config);

    // Check. There shouldn't be a 0xE1 byte in there
    a.checkEqual("no dummy byte", bytes.find(0xE1), bytes.size());
}

AFL_TEST("game.v3.HConfig:roundtrip", a)
{
    // Initialize with a hconfig file created by HOST/HCONFIG
    static const uint8_t FILE_DATA[] = {
        0x4b, 0x00, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x1e, 0x00, 0x01, 0x00, 0x0f, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x05, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x46, 0x00, 0xc8, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00,
        0x64, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00,
        0x00, 0x00, 0xc8, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00,
        0x64, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00, 0x64, 0x00,
        0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x01,
        0xc8, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x05, 0x00,
        0xc8, 0x00, 0x01, 0x00, 0x01, 0x00, 0x64, 0x00, 0x05, 0x00, 0x05, 0x00,
        0x96, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x05, 0x00, 0xc8, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x00, 0x14, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x03, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x05, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00,
        0xc8, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x64, 0x00,
        0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x28, 0x23,
        0x00, 0x00, 0x28, 0x23, 0x00, 0x00, 0x58, 0x1b, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x88, 0x13, 0x00, 0x00, 0x05, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 0x07, 0x00,
        0x00, 0x00, 0x01, 0x00
    };
    HConfig fig;
    static_assert(sizeof(FILE_DATA) == sizeof(fig), "sizeof fig");
    std::memcpy(&fig, FILE_DATA, sizeof(fig));

    // Create a as-blank-as-possible configuration
    HostConfiguration config;
    afl::base::Ref<Configuration::Enumerator_t> opts(config.getOptions());
    Configuration::OptionInfo_t optInfo;
    while (opts->getNextElement(optInfo)) {
        // "0" is a valid value for most options; catch errors anyway.
        try { config.setOption(optInfo.first, "0", game::config::ConfigurationOption::Game); }
        catch (...) { }
    }

    // Load hconfig
    game::v3::unpackHConfig(fig, sizeof(fig), config, game::config::ConfigurationOption::Game);

    // Verify some options
    a.checkEqual("01. RecycleRate",      config[HostConfiguration::RecycleRate](4), 75);
    a.checkEqual("02. SensorRange",      config[HostConfiguration::SensorRange](7), 200);
    a.checkEqual("03. MineSweepRange",   config[HostConfiguration::MineSweepRange](8), 5);
    a.checkEqual("04. AllowVPAFeatures", config[HostConfiguration::AllowVPAFeatures](), 1);

    // Save hconfig again
    game::v3::packHConfig(fig, config);
    a.checkEqualContent("11. data", afl::base::ConstBytes_t(afl::base::fromObject(fig)), afl::base::ConstBytes_t(FILE_DATA));
}

/*
 *  LokiDecloaksBirds scalar <> AntiCloakImmunity array
 */

AFL_TEST("game.v3.HConfig:LokiDecloaksBirds:on", a)
{
    HostConfiguration config;
    HConfig fig;
    afl::base::fromObject(fig).fill(0);

    // Set
    fig.LokiDecloaksBirds = 0;
    game::v3::unpackHConfig(fig, sizeof(fig), config, game::config::ConfigurationOption::Game);

    // Verify
    a.checkEqual("01", config[HostConfiguration::AntiCloakImmunity](1), 0);
    a.checkEqual("02", config[HostConfiguration::AntiCloakImmunity](2), 0);
    a.checkEqual("03", config[HostConfiguration::AntiCloakImmunity](3), 1);
    a.checkEqual("04", config[HostConfiguration::AntiCloakImmunity](4), 0);

    // Store
    game::v3::packHConfig(fig, config);
    a.checkEqual("11", fig.LokiDecloaksBirds, 0);
}

AFL_TEST("game.v3.HConfig:LokiDecloaksBirds:on:PlayerRace", a)
{
    HostConfiguration config;
    HConfig fig;
    afl::base::fromObject(fig).fill(0);
    config[HostConfiguration::PlayerRace].set("3,2,1,4");

    // Set
    fig.LokiDecloaksBirds = 0;
    game::v3::unpackHConfig(fig, sizeof(fig), config, game::config::ConfigurationOption::Game);

    // Verify
    a.checkEqual("01", config[HostConfiguration::AntiCloakImmunity](1), 1);
    a.checkEqual("02", config[HostConfiguration::AntiCloakImmunity](2), 0);
    a.checkEqual("03", config[HostConfiguration::AntiCloakImmunity](3), 0);
    a.checkEqual("04", config[HostConfiguration::AntiCloakImmunity](4), 0);

    // Store
    game::v3::packHConfig(fig, config);
    a.checkEqual("11", fig.LokiDecloaksBirds, 0);
}

AFL_TEST("game.v3.HConfig:LokiDecloaksBirds:on:PlayerRace:none", a)
{
    HostConfiguration config;
    HConfig fig;
    afl::base::fromObject(fig).fill(0);
    config[HostConfiguration::PlayerRace].set("1,1,1,4");

    // Set
    fig.LokiDecloaksBirds = 0;
    game::v3::unpackHConfig(fig, sizeof(fig), config, game::config::ConfigurationOption::Game);

    // Verify
    a.checkEqual("01", config[HostConfiguration::AntiCloakImmunity](1), 0);
    a.checkEqual("02", config[HostConfiguration::AntiCloakImmunity](2), 0);
    a.checkEqual("03", config[HostConfiguration::AntiCloakImmunity](3), 0);
    a.checkEqual("04", config[HostConfiguration::AntiCloakImmunity](4), 0);

    // Store
    game::v3::packHConfig(fig, config);
    a.checkEqual("11", fig.LokiDecloaksBirds, 0);
}

AFL_TEST("game.v3.HConfig:LokiDecloaksBirds:off", a)
{
    HostConfiguration config;
    HConfig fig;
    afl::base::fromObject(fig).fill(0);

    // Set
    fig.LokiDecloaksBirds = 1;
    game::v3::unpackHConfig(fig, sizeof(fig), config, game::config::ConfigurationOption::Game);

    // Verify
    a.checkEqual("01", config[HostConfiguration::AntiCloakImmunity](1), 0);
    a.checkEqual("02", config[HostConfiguration::AntiCloakImmunity](2), 0);
    a.checkEqual("03", config[HostConfiguration::AntiCloakImmunity](3), 0);
    a.checkEqual("04", config[HostConfiguration::AntiCloakImmunity](4), 0);

    // Store
    game::v3::packHConfig(fig, config);
    a.checkEqual("11", fig.LokiDecloaksBirds, 1);
}

/*
 *  ColonialFighterSweepRate scalar <> FighterSweepRate array
 */

AFL_TEST("game.v3.HConfig:ColonialFighterSweepRate", a)
{
    HostConfiguration config;
    HConfig fig;
    afl::base::fromObject(fig).fill(0);

    // Set
    fig.ColonialFighterSweepRate = 15;
    game::v3::unpackHConfig(fig, sizeof(fig), config, game::config::ConfigurationOption::Game);

    // Verify
    a.checkEqual("01", config[HostConfiguration::FighterSweepRate](1), 0);
    a.checkEqual("02", config[HostConfiguration::FighterSweepRate](2), 0);
    a.checkEqual("03", config[HostConfiguration::FighterSweepRate](10), 0);
    a.checkEqual("04", config[HostConfiguration::FighterSweepRate](11), 15);
    a.checkEqual("05", config[HostConfiguration::FighterSweepRate](12), 0);

    // Store
    game::v3::packHConfig(fig, config);
    a.checkEqual("11", fig.ColonialFighterSweepRate, 15);
}

AFL_TEST("game.v3.HConfig:ColonialFighterSweepRate:PlayerRace", a)
{
    HostConfiguration config;
    HConfig fig;
    afl::base::fromObject(fig).fill(0);
    config[HostConfiguration::PlayerRace].set("1,11,3");

    // Set
    fig.ColonialFighterSweepRate = 22;
    game::v3::unpackHConfig(fig, sizeof(fig), config, game::config::ConfigurationOption::Game);

    // Verify
    a.checkEqual("01", config[HostConfiguration::FighterSweepRate](1), 0);
    a.checkEqual("02", config[HostConfiguration::FighterSweepRate](2), 22);
    a.checkEqual("03", config[HostConfiguration::FighterSweepRate](3), 0);
    a.checkEqual("04", config[HostConfiguration::FighterSweepRate](10), 0);
    a.checkEqual("05", config[HostConfiguration::FighterSweepRate](11), 0);
    a.checkEqual("06", config[HostConfiguration::FighterSweepRate](12), 0);

    // Store
    game::v3::packHConfig(fig, config);
    a.checkEqual("11", fig.ColonialFighterSweepRate, 22);
}

AFL_TEST("game.v3.HConfig:ColonialFighterSweepRate:PlayerRace:none", a)
{
    HostConfiguration config;
    HConfig fig;
    afl::base::fromObject(fig).fill(0);
    config[HostConfiguration::PlayerRace].set("1,1,1");

    // Set
    fig.ColonialFighterSweepRate = 33;
    game::v3::unpackHConfig(fig, sizeof(fig), config, game::config::ConfigurationOption::Game);

    // Verify
    a.checkEqual("01", config[HostConfiguration::FighterSweepRate](1), 0);
    a.checkEqual("02", config[HostConfiguration::FighterSweepRate](2), 0);
    a.checkEqual("03", config[HostConfiguration::FighterSweepRate](3), 0);
    a.checkEqual("04", config[HostConfiguration::FighterSweepRate](10), 0);
    a.checkEqual("05", config[HostConfiguration::FighterSweepRate](11), 0);
    a.checkEqual("06", config[HostConfiguration::FighterSweepRate](12), 0);

    // Store
    game::v3::packHConfig(fig, config);
    a.checkEqual("11", fig.ColonialFighterSweepRate, 20);   // default value chosen
}
