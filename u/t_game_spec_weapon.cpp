/**
  *  \file u/t_game_spec_weapon.cpp
  *  \brief Test for game::spec::Weapon
  */

#include "game/spec/weapon.hpp"

#include "t_game_spec.hpp"

/** Simple test of getters/setters. */
void
TestGameSpecWeapon::testIt()
{
    game::spec::Weapon testee(game::spec::ComponentNameProvider::Beam, 7);

    // Initial state
    TS_ASSERT_EQUALS(testee.getKillPower(), 0);
    TS_ASSERT_EQUALS(testee.getDamagePower(), 0);
    TS_ASSERT_EQUALS(testee.getId(), 7);

    // Change
    testee.setKillPower(17);
    testee.setDamagePower(42);

    // Verify
    TS_ASSERT_EQUALS(testee.getKillPower(), 17);
    TS_ASSERT_EQUALS(testee.getDamagePower(), 42);
}
