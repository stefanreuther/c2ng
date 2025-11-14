/**
  *  \file test/game/spec/fightertest.cpp
  *  \brief Test for game::spec::Fighter
  */

#include "game/spec/fighter.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

using game::spec::Cost;
using game::config::ConfigurationOption;

AFL_TEST("game.spec.Fighter", a)
{
    // Player list with one player.
    game::PlayerList plList;
    game::Player* pl = plList.create(3);
    a.checkNonNull("01", pl);
    pl->setName(game::Player::AdjectiveName, "French");

    // Configuration. We check the default parameters.
    afl::base::Ref<game::config::HostConfiguration> rconfig = game::config::HostConfiguration::create();
    game::config::HostConfiguration& config = *rconfig;

    // Translator
    afl::string::NullTranslator tx;

    // Fighter to test
    {
        game::spec::Fighter                testee(3, config, plList, tx);
        a.checkEqual("11. getId",          testee.getId(), 3);
        a.checkEqual("12. getKillPower",   testee.getKillPower(), 2);
        a.checkEqual("13. getDamagePower", testee.getDamagePower(), 2);
        a.checkEqual("14. Tritanium",      testee.cost().get(Cost::Tritanium), 3);
        a.checkEqual("15. Duranium",       testee.cost().get(Cost::Duranium), 0);
        a.checkEqual("16. Molybdenum",     testee.cost().get(Cost::Molybdenum), 2);
        a.checkEqual("17. Money",          testee.cost().get(Cost::Money), 100);
        a.checkEqual("18. Supplies",       testee.cost().get(Cost::Supplies), 0);
    }

    // Change config
    config.setOption("FighterBeamKill", "9", ConfigurationOption::User);
    config.setOption("FighterBeamExplosive", "7", ConfigurationOption::User);

    // Test changed configuration
    {
        game::spec::Fighter testee(3, config, plList, tx);
        a.checkEqual("21. getId",          testee.getId(), 3);
        a.checkEqual("22. getKillPower",   testee.getKillPower(), 9);
        a.checkEqual("23. getDamagePower", testee.getDamagePower(), 7);
    }
}
