/**
  *  \file game/spec/info/utils.cpp
  *  \brief Utilities for common types in game::spec::info
  */

#include "game/spec/info/utils.hpp"

// Get name of FilterAttribute.
String_t
game::spec::info::toString(FilterAttribute att, afl::string::Translator& tx)
{
    switch (att) {
     case Range_CostD:              return tx("Duranium cost");
     case Range_CostM:              return tx("Molybdenum cost");
     case Range_CostMC:             return tx("Money cost");
     case Range_CostT:              return tx("Tritanium cost");
     case Range_DamagePower:        return tx("Damage power");
     case Range_HitOdds:            return tx("Hit odds");
     case Range_IsArmed:            return tx("Armed");
     case Range_IsDeathRay:         return tx("Type");
     case Range_KillPower:          return tx("Kill power");
     case Range_Mass:               return tx("Mass");
     case Range_MaxBeams:           return tx("Beams");
     case Range_MaxCargo:           return tx("Cargo");
     case Range_MaxCrew:            return tx("Crew");
     case Range_MaxEfficientWarp:   return tx("Max Efficient Warp");
     case Range_MaxFuel:            return tx("Fuel");
     case Range_MaxLaunchers:       return tx("Torpedo Launchers");
     case Range_NumBays:            return tx("Fighter Bays");
     case Range_NumEngines:         return tx("Engines");
     case Range_NumMinesSwept:      return tx("Mines swept");
     case Range_RechargeTime:       return tx("Recharge time");
     case Range_Tech:               return tx("Tech level");
     case Range_TorpCost:           return tx("Torpedo cost");
     case Range_Id:                 return tx("Id");
     case Value_Hull:               return tx("Hull");
     case Value_Player:             return tx("Player");
     case Value_Category:           return tx("Category");
     case Value_Origin:             return tx("From");
     case ValueRange_ShipAbility:   return tx("Has");
     case String_Name:              return tx("Name");
    }
    return String_t();
}

// Convert integer range to ExperienceLevelSet_t.
game::ExperienceLevelSet_t
game::spec::info::convertRangeToSet(IntRange_t r)
{
    game::ExperienceLevelSet_t set;
    if (!r.empty()) {
        set += game::ExperienceLevelSet_t::allUpTo(r.max());
        if (r.min() != 0) {
            set -= game::ExperienceLevelSet_t::allUpTo(r.min() - 1);
        }
    }
    return set;
}

// IntRange_t convertSetToRange(game::ExperienceLevelSet_t set)
// {
//     IntRange_t result;
//     int i = 0;
//     while (!set.empty()) {
//         if (set.contains(i)) {
//             result.include(i);
//             set -= i;
//         }
//         ++i;
//     }
//     return result;
// }

// Get available experience level range.
game::spec::info::IntRange_t
game::spec::info::getLevelRange(const Root& root)
{
    return IntRange_t(0, root.hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels]());
}

// Get available hull Id range.
game::spec::info::IntRange_t
game::spec::info::getHullRange(const ShipList& shipList)
{
    return IntRange_t(1, shipList.hulls().size());
}

// Get available player Id range.
game::spec::info::IntRange_t
game::spec::info::getPlayerRange(const Root& root)
{
    // As of 20200520, this will report one more than the highest player Id. Trim it.
    const PlayerList& list = root.playerList();
    int limit = list.size();
    while (limit > 0 && list.get(limit) == 0) {
        --limit;
    }

    return IntRange_t(1, limit);
}

// Get default range for a filter attribute.
game::spec::info::IntRange_t
game::spec::info::getAttributeRange(FilterAttribute att)
{
    switch (att) {
     case Range_Tech:
        return IntRange_t(1, 10);
     default:
        return IntRange_t(0, 20000);
    }
}
