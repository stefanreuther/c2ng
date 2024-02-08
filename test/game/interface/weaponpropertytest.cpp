/**
  *  \file test/game/interface/weaponpropertytest.cpp
  *  \brief Test for game::interface::WeaponProperty
  */

#include "game/interface/weaponproperty.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "interpreter/test/valueverifier.hpp"
#include <memory>

using interpreter::test::verifyNewInteger;

/** Test it: exercise all combinations. */
AFL_TEST("game.interface.WeaponProperty", a)
{
    game::spec::Weapon w(game::spec::ComponentNameProvider::Hull, 0);
    w.setKillPower(13);
    w.setDamagePower(17);

    game::config::HostConfiguration configAC0;
    game::config::HostConfiguration configAC1;
    configAC0[game::config::HostConfiguration::AllowAlternativeCombat].set(0);
    configAC1[game::config::HostConfiguration::AllowAlternativeCombat].set(1);

    // As beam
    verifyNewInteger(a("beam kill p0"), getWeaponProperty(w, game::interface::iwpKill, configAC0, false), 13);
    verifyNewInteger(a("beam kill p1"), getWeaponProperty(w, game::interface::iwpKill, configAC1, false), 13);
    verifyNewInteger(a("beam damage p0"), getWeaponProperty(w, game::interface::iwpDamage, configAC0, false), 17);
    verifyNewInteger(a("beam damage p1"), getWeaponProperty(w, game::interface::iwpDamage, configAC1, false), 17);

    // As torpedo
    verifyNewInteger(a("torp kill p0"), getWeaponProperty(w, game::interface::iwpKill, configAC0, true), 26);
    verifyNewInteger(a("torp kill p1"), getWeaponProperty(w, game::interface::iwpKill, configAC1, true), 13);
    verifyNewInteger(a("torp damage p0"), getWeaponProperty(w, game::interface::iwpDamage, configAC0, true), 34);
    verifyNewInteger(a("torp damage p1"), getWeaponProperty(w, game::interface::iwpDamage, configAC1, true), 17);
}
