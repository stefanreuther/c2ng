/**
  *  \file u/t_game_sim_ability.cpp
  *  \brief Test for game::sim::Ability
  */

#include "game/sim/ability.hpp"

#include "t_game_sim.hpp"

/** Test well-formedness of header file.
    This is just an enum, so we can't test much more than that. */
void
TestGameSimAbility::testIt()
{
    game::sim::Ability testee = game::sim::ElusiveAbility;
    (void) testee;
}

