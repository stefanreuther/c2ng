/**
  *  \file u/t_game_config_hostconfiguration.cpp
  *  \brief Test for game::config::HostConfiguration
  */

#include "game/config/hostconfiguration.hpp"

#include "t_game_config.hpp"

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
