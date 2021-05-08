/**
  *  \file u/t_game_sim_ability.cpp
  *  \brief Test for game::sim::Ability
  */

#include "game/sim/ability.hpp"

#include "t_game_sim.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test well-formedness of header file.
    This is just an enum, so we can't test much more than that. */
void
TestGameSimAbility::testIt()
{
    game::sim::Ability testee = game::sim::ElusiveAbility;
    TS_ASSERT(testee >= game::sim::FIRST_ABILITY);
    TS_ASSERT(testee <= game::sim::LAST_ABILITY);
}

void
TestGameSimAbility::testToString()
{
    afl::string::NullTranslator tx;

    // Verify all abilities stringify
    for (int i = game::sim::FIRST_ABILITY; i <= game::sim::LAST_ABILITY; ++i) {
        TS_ASSERT_LESS_THAN_EQUALS(2U, toString(game::sim::Ability(i), tx).size());
    }

    // Verify concrete stringifications
    TS_ASSERT_EQUALS(toString(game::sim::CommanderAbility, tx), "Commander");
    TS_ASSERT_EQUALS(toString(game::sim::ElusiveAbility, tx), "Elusive");

    TS_ASSERT_EQUALS(toString(game::sim::Abilities_t(), tx), "none");
    TS_ASSERT_EQUALS(toString(game::sim::Abilities_t() + game::sim::CommanderAbility, tx), "Commander");
    TS_ASSERT_EQUALS(toString(game::sim::Abilities_t() + game::sim::CommanderAbility + game::sim::ElusiveAbility, tx), "Commander, Elusive");
}

