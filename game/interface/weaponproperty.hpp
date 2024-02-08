/**
  *  \file game/interface/weaponproperty.hpp
  *  \brief Enum game::interface::WeaponProperty
  */
#ifndef C2NG_GAME_INTERFACE_WEAPONPROPERTY_HPP
#define C2NG_GAME_INTERFACE_WEAPONPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/spec/weapon.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace interface {

    /** Generic weapon property. */
    enum WeaponProperty {
        iwpKill,
        iwpDamage
    };

    /** Get weapon property.
        @param w       Weapon
        @param iwp     Property
        @param config  Host configuration (for hasDoubleTorpedoPower())
        @param isTorpedo  true if torpedo */
    afl::data::Value* getWeaponProperty(const game::spec::Weapon& w, WeaponProperty iwp,
                                        const game::config::HostConfiguration& config,
                                        bool isTorpedo);

} }

#endif
