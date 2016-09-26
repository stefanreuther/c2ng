/**
  *  \file game/interface/weaponproperty.cpp
  */

#include "game/interface/weaponproperty.hpp"
#include "interpreter/values.hpp"

using interpreter::makeIntegerValue;

// /** Get property of a weapon. */
afl::data::Value*
game::interface::getWeaponProperty(const game::spec::Weapon& w, WeaponProperty iwp,
                                   const game::config::HostConfiguration& config,
                                   const game::HostVersion& host,
                                   bool isTorpedo)
{
    // ex int/if/specif.h:getWeaponProperty
    /* We're doubling torpedo bang/kill values when appropriate */
    int factor;
    if (isTorpedo && host.hasDoubleTorpedoPower(config)) {
        factor = 2;
    } else {
        factor = 1;
    }

    switch (iwp) {
     case iwpKill:
        /* @q Kill:Int (Beam Property, Torpedo Property)
           Anti-life power of this weapon. */
        return makeIntegerValue(factor * w.getKillPower());
     case iwpDamage:
        /* @q Damage:Int (Beam Property, Torpedo Property)
           Explosive power of this weapon. */
        return makeIntegerValue(factor * w.getDamagePower());
    }
    return 0;
}
