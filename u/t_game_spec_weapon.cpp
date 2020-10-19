/**
  *  \file u/t_game_spec_weapon.cpp
  *  \brief Test for game::spec::Weapon
  */

#include "game/spec/weapon.hpp"

#include "t_game_spec.hpp"
#include "game/hostversion.hpp"

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

/** Test isDeathRay(). */
void
TestGameSpecWeapon::testDeathRay()
{
    game::spec::Weapon testee(game::spec::ComponentNameProvider::Beam, 3);
    testee.setKillPower(99);
    testee.setDamagePower(0);

    const game::HostVersion p4(game::HostVersion::PHost, MKVERSION(4, 0, 0));
    const game::HostVersion p3(game::HostVersion::PHost, MKVERSION(3, 2, 0));
    const game::HostVersion t(game::HostVersion::Host, MKVERSION(3, 22, 0));

    TS_ASSERT_EQUALS(testee.isDeathRay(p4), true);
    TS_ASSERT_EQUALS(testee.isDeathRay(p3), false);
    TS_ASSERT_EQUALS(testee.isDeathRay(t), false);

    testee.setKillPower(99);
    testee.setDamagePower(100);
    
    TS_ASSERT_EQUALS(testee.isDeathRay(p4), false);
    TS_ASSERT_EQUALS(testee.isDeathRay(p3), false);
    TS_ASSERT_EQUALS(testee.isDeathRay(t), false);
}
