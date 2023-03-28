/**
  *  \file u/t_game_interface_weaponproperty.cpp
  *  \brief Test for game::interface::WeaponProperty
  */

#include <memory>
#include "game/interface/weaponproperty.hpp"

#include "t_game_interface.hpp"
#include "game/config/hostconfiguration.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewInteger;

/** Test it: exercise all combinations. */
void
TestGameInterfaceWeaponProperty::testIt()
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
    verifyNewInteger("beam kill t0", getWeaponProperty(w, game::interface::iwpKill, configAC0, thost, false), 13);
    verifyNewInteger("beam kill t1", getWeaponProperty(w, game::interface::iwpKill, configAC1, thost, false), 13);
    verifyNewInteger("beam kill p0", getWeaponProperty(w, game::interface::iwpKill, configAC0, phost, false), 13);
    verifyNewInteger("beam kill p1", getWeaponProperty(w, game::interface::iwpKill, configAC1, phost, false), 13);
    verifyNewInteger("beam damage t0", getWeaponProperty(w, game::interface::iwpDamage, configAC0, thost, false), 17);
    verifyNewInteger("beam damage t1", getWeaponProperty(w, game::interface::iwpDamage, configAC1, thost, false), 17);
    verifyNewInteger("beam damage p0", getWeaponProperty(w, game::interface::iwpDamage, configAC0, phost, false), 17);
    verifyNewInteger("beam damage p1", getWeaponProperty(w, game::interface::iwpDamage, configAC1, phost, false), 17);

    // As torpedo
    verifyNewInteger("torp kill t0", getWeaponProperty(w, game::interface::iwpKill, configAC0, thost, true), 26);
    verifyNewInteger("torp kill t1", getWeaponProperty(w, game::interface::iwpKill, configAC1, thost, true), 26);
    verifyNewInteger("torp kill p0", getWeaponProperty(w, game::interface::iwpKill, configAC0, phost, true), 26);
    verifyNewInteger("torp kill p1", getWeaponProperty(w, game::interface::iwpKill, configAC1, phost, true), 13);
    verifyNewInteger("torp damage t0", getWeaponProperty(w, game::interface::iwpDamage, configAC0, thost, true), 34);
    verifyNewInteger("torp damage t1", getWeaponProperty(w, game::interface::iwpDamage, configAC1, thost, true), 34);
    verifyNewInteger("torp damage p0", getWeaponProperty(w, game::interface::iwpDamage, configAC0, phost, true), 34);
    verifyNewInteger("torp damage p1", getWeaponProperty(w, game::interface::iwpDamage, configAC1, phost, true), 17);
}

