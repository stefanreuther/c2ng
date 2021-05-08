/**
  *  \file u/t_game_config_userconfiguration.cpp
  *  \brief Test for game::config::UserConfiguration
  */

#include "game/config/userconfiguration.hpp"

#include "t_game_config.hpp"
#include "game/types.hpp"
#include "game/config/markeroption.hpp"

/** Test defaults.
    This tests initialisation. */
void
TestGameConfigUserConfiguration::testDefaults()
{
    game::config::UserConfiguration testee;
    TS_ASSERT_EQUALS(testee[testee.Display_ThousandsSep](), 1);
    TS_ASSERT_EQUALS(testee[testee.Display_Clans](), 0);
}

/** Test getGameType(). */
void
TestGameConfigUserConfiguration::testGameType()
{
    // Uninitialized. Game type must be empty.
    {
        game::config::UserConfiguration testee;
        TS_ASSERT_EQUALS(testee.getGameType(), "");
        TS_ASSERT(testee.getOptionByName("game.type") == 0);
    }

    // Name has been set
    {
        game::config::UserConfiguration testee;
        testee.setOption("game.type", "foo", game::config::ConfigurationOption::User);
        TS_ASSERT_EQUALS(testee.getGameType(), "foo");
        TS_ASSERT(testee.getOptionByName("game.type") != 0);
    }
}

void
TestGameConfigUserConfiguration::testFormat()
{
    // Defaults: thousands separators, but no clans
    {
        game::config::UserConfiguration testee;
        TS_ASSERT_EQUALS(testee.formatNumber(1), "1");
        TS_ASSERT_EQUALS(testee.formatNumber(1000), "1,000");
        TS_ASSERT_EQUALS(testee.formatNumber(-1000), "-1,000");
        TS_ASSERT_EQUALS(testee.formatNumber(1000000), "1,000,000");
        TS_ASSERT_EQUALS(testee.formatNumber(-100000), "-100,000");
        TS_ASSERT_EQUALS(testee.formatPopulation(33), "3,300");
        TS_ASSERT_EQUALS(testee.formatPopulation(334455), "33,445,500");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t(2000)), "2,000");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t()), "");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t(2000)), "200,000");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t()), "");
    }

    // No thousands separators
    {
        game::config::UserConfiguration testee;
        testee[testee.Display_ThousandsSep].set(0);
        TS_ASSERT_EQUALS(testee.formatNumber(1), "1");
        TS_ASSERT_EQUALS(testee.formatNumber(1000), "1000");
        TS_ASSERT_EQUALS(testee.formatNumber(-1000), "-1000");
        TS_ASSERT_EQUALS(testee.formatNumber(1000000), "1000000");
        TS_ASSERT_EQUALS(testee.formatNumber(-100000), "-100000");
        TS_ASSERT_EQUALS(testee.formatPopulation(33), "3300");
        TS_ASSERT_EQUALS(testee.formatPopulation(334455), "33445500");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t(2000)), "2000");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t()), "");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t(2000)), "200000");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t()), "");
    }

    // Clans
    {
        game::config::UserConfiguration testee;
        testee[testee.Display_Clans].set(1);
        TS_ASSERT_EQUALS(testee.formatPopulation(33), "33c");
        TS_ASSERT_EQUALS(testee.formatPopulation(334455), "334,455c");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t(2000)), "2,000c");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t()), "");
    }
}

/** Test getCannedMarker().
    A: create UserConfiguration. Call getCannedMarker() with valid and invalid index.
    E: invalid index returns null; valid index returns expected value */
void
TestGameConfigUserConfiguration::testCannedMarker()
{
    game::config::UserConfiguration testee;
    TS_ASSERT(testee.getCannedMarker(-1) == 0);
    TS_ASSERT(testee.getCannedMarker(1000) == 0);

    game::config::MarkerOption* opt = testee.getCannedMarker(2);
    TS_ASSERT(opt != 0);
    TS_ASSERT_EQUALS(opt->getColor(), 9);
    TS_ASSERT_EQUALS(opt->getMarkerKind(), 1);
    TS_ASSERT_EQUALS(opt->getNote(), "");
}
