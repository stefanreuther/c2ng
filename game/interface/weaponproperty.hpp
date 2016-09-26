/**
  *  \file game/interface/weaponproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_WEAPONPROPERTY_HPP
#define C2NG_GAME_INTERFACE_WEAPONPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/spec/weapon.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"

namespace game { namespace interface {

    // /** Property for a GWeapon. */
    enum WeaponProperty {
        iwpKill,
        iwpDamage
        // TorpCost? PCC 1.x doesn't have it.
    };

    afl::data::Value* getWeaponProperty(const game::spec::Weapon& w, WeaponProperty iwp,
                                        const game::config::HostConfiguration& config,
                                        const game::HostVersion& host,
                                        bool isTorpedo);


} }

#endif
