/**
  *  \file u/t_game_spec_fighter.cpp
  *  \brief Test for game::spec::Fighter
  */

#include "game/spec/fighter.hpp"

#include "t_game_spec.hpp"
#include "afl/string/nulltranslator.hpp"

using game::spec::Cost;
using game::config::ConfigurationOption;

void
TestGameSpecFighter::testIt()
{
    // Player list with one player.
    game::PlayerList plList;
    game::Player* pl = plList.create(3);
    TS_ASSERT(pl != 0);
    pl->setName(game::Player::AdjectiveName, "French");

    // Configuration. We check the default parameters.
    game::config::HostConfiguration config;

    // Translator
    afl::string::NullTranslator tx;

    // Fighter to test
    {
        game::spec::Fighter testee(3, config, plList, tx);
        TS_ASSERT_EQUALS(testee.getId(), 3);
        TS_ASSERT_EQUALS(testee.getKillPower(), 2);
        TS_ASSERT_EQUALS(testee.getDamagePower(), 2);
        TS_ASSERT_EQUALS(testee.cost().get(Cost::Tritanium), 3);
        TS_ASSERT_EQUALS(testee.cost().get(Cost::Duranium), 0);
        TS_ASSERT_EQUALS(testee.cost().get(Cost::Molybdenum), 2);
        TS_ASSERT_EQUALS(testee.cost().get(Cost::Money), 100);
        TS_ASSERT_EQUALS(testee.cost().get(Cost::Supplies), 0);
    }

    // Change config
    config.setOption("FighterBeamKill", "9", ConfigurationOption::User);
    config.setOption("FighterBeamExplosive", "7", ConfigurationOption::User);

    // Test changed configuration
    {
        game::spec::Fighter testee(3, config, plList, tx);
        TS_ASSERT_EQUALS(testee.getId(), 3);
        TS_ASSERT_EQUALS(testee.getKillPower(), 9);
        TS_ASSERT_EQUALS(testee.getDamagePower(), 7);
    }
}

