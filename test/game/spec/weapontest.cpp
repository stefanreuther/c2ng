/**
  *  \file test/game/spec/weapontest.cpp
  *  \brief Test for game::spec::Weapon
  */

#include "game/spec/weapon.hpp"

#include "afl/test/testrunner.hpp"
#include "game/hostversion.hpp"

/** Simple test of getters/setters. */
AFL_TEST("game.spec.Weapon:basics", a)
{
    game::spec::Weapon testee(game::spec::ComponentNameProvider::Beam, 7);

    // Initial state
    a.checkEqual("01. getKillPower", testee.getKillPower(), 0);
    a.checkEqual("02. getDamagePower", testee.getDamagePower(), 0);
    a.checkEqual("03. getId", testee.getId(), 7);

    // Change
    testee.setKillPower(17);
    testee.setDamagePower(42);

    // Verify
    a.checkEqual("11. getKillPower", testee.getKillPower(), 17);
    a.checkEqual("12. getDamagePower", testee.getDamagePower(), 42);
}

/** Test isDeathRay(). */
AFL_TEST("game.spec.Weapon:isDeathRay", a)
{
    game::spec::Weapon testee(game::spec::ComponentNameProvider::Beam, 3);
    testee.setKillPower(99);
    testee.setDamagePower(0);

    const game::HostVersion p4(game::HostVersion::PHost, MKVERSION(4, 0, 0));
    const game::HostVersion p3(game::HostVersion::PHost, MKVERSION(3, 2, 0));
    const game::HostVersion t(game::HostVersion::Host, MKVERSION(3, 22, 0));

    a.checkEqual("01", testee.isDeathRay(p4), true);
    a.checkEqual("02", testee.isDeathRay(p3), false);
    a.checkEqual("03", testee.isDeathRay(t), false);

    testee.setKillPower(99);
    testee.setDamagePower(100);

    a.checkEqual("11", testee.isDeathRay(p4), false);
    a.checkEqual("12", testee.isDeathRay(p3), false);
    a.checkEqual("13", testee.isDeathRay(t), false);
}
