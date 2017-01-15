/**
  *  \file game/sim/ability.hpp
  */
#ifndef C2NG_GAME_SIM_ABILITY_HPP
#define C2NG_GAME_SIM_ABILITY_HPP

namespace game { namespace sim {

    enum Ability {                   // ex GSimFunction
        PlanetImmunityAbility,       // ex sf_PlanetImmunity,
        FullWeaponryAbility,         // ex sf_FullWeaponry,
        CommanderAbility,            // ex sf_Commander,
        TripleBeamKillAbility,       // ex sf_TripleBeamKill,
        DoubleBeamChargeAbility,     // ex sf_DoubleBeamCharge,
        DoubleTorpedoChargeAbility,  // ex sf_DoubleTorpCharge,
        ElusiveAbility,              // ex sf_Elusive,
        SquadronAbility              // ex sf_Squadron
    };

} }

#endif
