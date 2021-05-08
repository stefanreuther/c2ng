/**
  *  \file game/sim/ability.cpp
  *  \brief Enum game::sim::Ability
  */

#include "game/sim/ability.hpp"
#include "util/string.hpp"

String_t
game::sim::toString(Ability a, afl::string::Translator& tx)
{
    // ex client/dialogs/simfunction.cc:LABELS
    switch (a) {
     case PlanetImmunityAbility:       return tx("Planet Immunity");
     case FullWeaponryAbility:         return tx("Full Weaponry");
     case CommanderAbility:            return tx("Commander");
     case TripleBeamKillAbility:       return tx("3\xC3\x97 Beam Kill");
     case DoubleBeamChargeAbility:     return tx("2\xC3\x97 Beam Charge");
     case DoubleTorpedoChargeAbility:  return tx("2\xC3\x97 Torp Charge");
     case ElusiveAbility:              return tx("Elusive");
     case SquadronAbility:             return tx("Squadron");
     case ShieldGeneratorAbility:      return tx("Shield Generator");
     case CloakedBaysAbility:          return tx("Cloaked Fighter Bays");
    }
    return String_t();
}

String_t
game::sim::toString(Abilities_t as, afl::string::Translator& tx)
{
    // ex describeFunctions
    String_t result;
    for (int i = FIRST_ABILITY; i <= LAST_ABILITY; ++i) {
        Ability a = Ability(i);
        if (as.contains(a)) {
            util::addListItem(result, ", ", toString(a, tx));
        }
    }
    if (result.empty()) {
        result = tx("none");
    }
    return result;
}
