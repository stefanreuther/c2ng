/**
  *  \file u/t_game_config_hostconfiguration.cpp
  *  \brief Test for game::config::HostConfiguration
  */

#include "game/config/hostconfiguration.hpp"

#include "t_game_config.hpp"
#include "game/config/aliasoption.hpp"

/** Test race number accesses. */
void
TestGameConfigHostConfiguration::testRace()
{
    game::config::HostConfiguration testee;

    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(1), 1);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(5), 5);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(20), 20);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(1000), 1000);

    testee[testee.PlayerRace].set(5, 3);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(1), 1);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(5), 3);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(20), 20);
    TS_ASSERT_EQUALS(testee.getPlayerRaceNumber(1000), 1000);
}

/** Test configuration of aliases. */
void
TestGameConfigHostConfiguration::testAlias()
{
    using game::config::HostConfiguration;
    HostConfiguration testee;

    // Get enumerator
    afl::base::Ptr<HostConfiguration::Enumerator_t> e = testee.getOptions();
    TS_ASSERT(e.get() != 0);

    // Count and verify options
    int numOptions = 0;
    int numAliases = 0;
    HostConfiguration::OptionInfo_t info;
    while (e->getNextElement(info)) {
        // Verify base properties
        TS_ASSERT(info.second != 0);
        TS_ASSERT(!info.first.empty());

        if (game::config::AliasOption* opt = dynamic_cast<game::config::AliasOption*>(info.second)) {
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
