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

    game::HostVersion thost(game::HostVersion::Host, MKVERSION(3,22,0));
    game::HostVersion phost(game::HostVersion::PHost, MKVERSION(4,0,0));

    // As beam
    verifyNewInteger(a("beam kill t0"), getWeaponProperty(w, game::interface::iwpKill, configAC0, thost, false), 13);
    verifyNewInteger(a("beam kill t1"), getWeaponProperty(w, game::interface::iwpKill, configAC1, thost, false), 13);
    verifyNewInteger(a("beam kill p0"), getWeaponProperty(w, game::interface::iwpKill, configAC0, phost, false), 13);
    verifyNewInteger(a("beam kill p1"), getWeaponProperty(w, game::interface::iwpKill, configAC1, phost, false), 13);
    verifyNewInteger(a("beam damage t0"), getWeaponProperty(w, game::interface::iwpDamage, configAC0, thost, false), 17);
    verifyNewInteger(a("beam damage t1"), getWeaponProperty(w, game::interface::iwpDamage, configAC1, thost, false), 17);
    verifyNewInteger(a("beam damage p0"), getWeaponProperty(w, game::interface::iwpDamage, configAC0, phost, false), 17);
    verifyNewInteger(a("beam damage p1"), getWeaponProperty(w, game::interface::iwpDamage, configAC1, phost, false), 17);

    // As torpedo
    verifyNewInteger(a("torp kill t0"), getWeaponProperty(w, game::interface::iwpKill, configAC0, thost, true), 26);
    verifyNewInteger(a("torp kill t1"), getWeaponProperty(w, game::interface::iwpKill, configAC1, thost, true), 26);
    verifyNewInteger(a("torp kill p0"), getWeaponProperty(w, game::interface::iwpKill, configAC0, phost, true), 26);
    verifyNewInteger(a("torp kill p1"), getWeaponProperty(w, game::interface::iwpKill, configAC1, phost, true), 13);
    verifyNewInteger(a("torp damage t0"), getWeaponProperty(w, game::interface::iwpDamage, configAC0, thost, true), 34);
    verifyNewInteger(a("torp damage t1"), getWeaponProperty(w, game::interface::iwpDamage, configAC1, thost, true), 34);
    verifyNewInteger(a("torp damage p0"), getWeaponProperty(w, game::interface::iwpDamage, configAC0, phost, true), 34);
    verifyNewInteger(a("torp damage p1"), getWeaponProperty(w, game::interface::iwpDamage, configAC1, phost, true), 17);
}
