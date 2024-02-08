/**
  *  \file game/interface/weaponproperty.cpp
  *  \brief Enum game::interface::WeaponProperty
  */

#include "game/interface/weaponproperty.hpp"
#include "interpreter/values.hpp"

using interpreter::makeIntegerValue;

afl::data::Value*
game::interface::getWeaponProperty(const game::spec::Weapon& w, WeaponProperty iwp,
                                   const game::config::HostConfiguration& config,
                                   bool isTorpedo)
{
    // ex int/if/specif.h:getWeaponProperty
    /* We're doubling torpedo bang/kill values when appropriate */
    int factor;
    if (isTorpedo && config.hasDoubleTorpedoPower()) {
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
