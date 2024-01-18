/**
  *  \file test/game/sim/abilitytest.cpp
  *  \brief Test for game::sim::Ability
  */

#include "game/sim/ability.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Test well-formedness of header file.
    This is just an enum, so we can't test much more than that. */
AFL_TEST("game.sim.Ability:range", a)
{
    game::sim::Ability testee = game::sim::ElusiveAbility;
    a.check("01", testee >= game::sim::FIRST_ABILITY);
    a.check("02", testee <= game::sim::LAST_ABILITY);
}

AFL_TEST("game.sim.Ability:toString", a)
{
    afl::string::NullTranslator tx;

    // Verify all abilities stringify
    for (int i = game::sim::FIRST_ABILITY; i <= game::sim::LAST_ABILITY; ++i) {
        a.check("01. size", toString(game::sim::Ability(i), tx).size() >= 2);
    }

    // Verify concrete stringifications
    a.checkEqual("11. toString", toString(game::sim::CommanderAbility, tx), "Commander");
    a.checkEqual("12. toString", toString(game::sim::ElusiveAbility, tx), "Elusive");

    a.checkEqual("21. toString", toString(game::sim::Abilities_t(), tx), "none");
    a.checkEqual("22. toString", toString(game::sim::Abilities_t() + game::sim::CommanderAbility, tx), "Commander");
    a.checkEqual("23. toString", toString(game::sim::Abilities_t() + game::sim::CommanderAbility + game::sim::ElusiveAbility, tx), "Commander, Elusive");
}
