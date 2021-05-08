/**
  *  \file game/sim/ability.hpp
  *  \brief Enum game::sim::Ability
  */
#ifndef C2NG_GAME_SIM_ABILITY_HPP
#define C2NG_GAME_SIM_ABILITY_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace sim {

    /** A ship ability as handled by the simulator.
        This is just a subset, because not all abilities are relevant to combat. */
    enum Ability {                   // ex GSimFunction
        PlanetImmunityAbility,       // ex sf_PlanetImmunity,
        FullWeaponryAbility,         // ex sf_FullWeaponry,
        CommanderAbility,            // ex sf_Commander,
        TripleBeamKillAbility,       // ex sf_TripleBeamKill,
        DoubleBeamChargeAbility,     // ex sf_DoubleBeamCharge,
        DoubleTorpedoChargeAbility,  // ex sf_DoubleTorpCharge,
        ElusiveAbility,              // ex sf_Elusive,
        SquadronAbility,             // ex sf_Squadron
        ShieldGeneratorAbility,      // ex sf_ShieldGenerator
        CloakedBaysAbility           // ex sf_CloakedBays
    };

    const Ability FIRST_ABILITY = PlanetImmunityAbility;
    const Ability LAST_ABILITY = CloakedBaysAbility;

    /** Set of abilities. */
    typedef afl::bits::SmallSet<Ability> Abilities_t;

    /** Describe ability.
        \param a Ability
        \param tx Translator
        \return Ability name */
    String_t toString(Ability a, afl::string::Translator& tx);

    /** Describe set of abilities.
        \param as Ability set
        \param tx Translator
        \return List of ability names, "none" if set is empty */
    String_t toString(Abilities_t as, afl::string::Translator& tx);

} }

#endif
