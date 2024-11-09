/**
  *  \file game/spec/shiplist.cpp
  *  \brief Class game::spec::ShipList
  */

#include "game/spec/shiplist.hpp"

// Constructor.
game::spec::ShipList::ShipList()
    : sig_change(),
      m_beams(),
      m_engines(),
      m_launchers(),
      m_hulls(),
      m_basicHullFunctions(),
      m_modifiedHullFunctions(),
      m_racialAbilities(),
      m_advantages(),
      m_hullAssignments(),
      m_componentNamer(),
      m_friendlyCodes(),
      m_missions(MissionList::create())
{ }

// Destructor.
game::spec::ShipList::~ShipList()
{ }

// Get component, given a reference.
const game::spec::Component*
game::spec::ShipList::getComponent(Reference ref) const
{
    switch (ref.getType()) {
     case Reference::Null:
     case Reference::Special:
     case Reference::Player:
     case Reference::MapLocation:
     case Reference::Ship:
     case Reference::Planet:
     case Reference::Starbase:
     case Reference::IonStorm:
     case Reference::Minefield:
     case Reference::Ufo:
        return 0;

     case Reference::Hull:
        return hulls().get(ref.getId());
     case Reference::Engine:
        return engines().get(ref.getId());
     case Reference::Beam:
        return beams().get(ref.getId());
     case Reference::Torpedo:
        return launchers().get(ref.getId());
    }
    return 0;
}

// Get a component, given area and Id.
const game::spec::Component*
game::spec::ShipList::getComponent(TechLevel area, Id_t id) const
{
    switch (area) {
     case HullTech:
        return hulls().get(id);
     case EngineTech:
        return engines().get(id);
     case BeamTech:
        return beams().get(id);
     case TorpedoTech:
        return launchers().get(id);
    }
    return 0;
}

// Find racial abilities.
void
game::spec::ShipList::findRacialAbilities(const game::config::HostConfiguration& config)
{
    // ex GHull::findRacialAbilities, hullfunc.pas:FindRacialAbilities
    // Sanity check
    Hull* referenceHull = hulls().findNext(0);
    if (!referenceHull) {
        return;
    }
    HullFunctionAssignmentList& referenceAssignments = referenceHull->getHullFunctions(true /* assigned to hull */);

    size_t i = referenceAssignments.getNumEntries();
    while (i > 0) {
        // Go backward because we will be deleting things from referenceAssignments
        --i;

        // Hull #1 has some hull function for a particular set of players.
        if (const HullFunctionAssignmentList::Entry* entry = referenceAssignments.getEntryByIndex(i)) {
            const ModifiedHullFunctionList::Function_t function = entry->m_function;
            PlayerSet_t players =
                HullFunction::getDefaultAssignment(int32_t(entry->m_function), config, *referenceHull)
                + entry->m_addedPlayers
                - entry->m_removedPlayers;

            // Check all other hulls and check who of them has that function, too.
            for (Hull* otherHull = hulls().findNext(referenceHull->getId());
                 otherHull != 0 && !players.empty();
                 otherHull = hulls().findNext(otherHull->getId()))
            {
                if (const HullFunctionAssignmentList::Entry* otherEntry = otherHull->getHullFunctions(true).findEntry(function)) {
                    players &= (HullFunction::getDefaultAssignment(int32_t(function), config, *otherHull)
                                + otherEntry->m_addedPlayers
                                - otherEntry->m_removedPlayers);
                } else {
                    players.clear();
                    break;
                }
            }

            // players now contains all players that have this function on all ships.
            // In this case, add it as racial ability and stub it out for the ships.
            // We only remove assignments that match our racial ability completely,
            // so that the information "SSD has planet-immunity for all races" remains available
            // even if Klingons/Rebels have that as a racial ability.
            // Otherwise, we would list the SSD as immune for everyone but Kli/Reb.
            // The disadvantage is that things like "races 1,2,3 have X on all ships,
            // except for race 1 on ship Z" should normally better be represented as racial abilities for 2 and 3.
            if (!players.empty()) {
                m_racialAbilities.change(function, players, PlayerSet_t());
                for (Hull* hull = hulls().findNext(0); hull != 0; hull = hulls().findNext(hull->getId())) {
                    if (const HullFunctionAssignmentList::Entry* entry = hull->getHullFunctions(true).findEntry(function)) {
                        if (players == (HullFunction::getDefaultAssignment(int32_t(function), config, *hull)
                                        - entry->m_addedPlayers
                                        + entry->m_removedPlayers))
                        {
                            // exact match
                            hull->getHullFunctions(true).removeEntry(function);
                        }
                    }
                }
            }
        }
    }
}

// Enumerate all hull functions related to a hull.
void
game::spec::ShipList::enumerateHullFunctions(HullFunctionList& result,
                                             int hullNr,
                                             const game::config::HostConfiguration& config,
                                             PlayerSet_t playerLimit,
                                             ExperienceLevelSet_t levelLimit,
                                             bool includeNewShip,
                                             bool includeRacialAbilities) const
{
    // ex GHull::enumerateHullFunctions
    if (const Hull* hull = hulls().get(hullNr)) {
        if (includeRacialAbilities) {
            m_racialAbilities.getAll(result, modifiedHullFunctions(), config, *hull, playerLimit, levelLimit, HullFunction::AssignedToRace);
        }
        if (includeNewShip) {
            hull->getHullFunctions(false).getAll(result, modifiedHullFunctions(), config, *hull, playerLimit, levelLimit, HullFunction::AssignedToShip);
        }
        hull->getHullFunctions(true).getAll(result, modifiedHullFunctions(), config, *hull, playerLimit, levelLimit, HullFunction::AssignedToHull);
    }
}

// Get specimen hull for a hull function.
const game::spec::Hull*
game::spec::ShipList::findSpecimenHullForFunction(int basicFunctionId, const game::config::HostConfiguration& config, PlayerSet_t playerLimit, PlayerSet_t buildLimit, bool unique) const
{
    // ex client/dlg-tax.cc:findSpecimenHull
    const Hull* result = 0;
    for (const Hull* candidate = hulls().findNext(0); candidate != 0; candidate = hulls().findNext(candidate->getId())) {
        if (buildLimit.empty() || hullAssignments().getPlayersForHull(config, candidate->getId()).containsAnyOf(buildLimit)) {
            PlayerSet_t set = candidate->getHullFunctions(true)
                .getPlayersThatCan(basicFunctionId, modifiedHullFunctions(), basicHullFunctions(), config, *candidate, ExperienceLevelSet_t(0), true);
            if (set.contains(playerLimit)) {
                if (result == 0) {
                    // First candidate
                    result = candidate;
                    if (!unique) {
                        break;
                    }
                } else {
                    // Ambiguous
                    result = 0;
                    break;
                }
            }
        }
    }
    return result;
}

// Get player mask for special function.
game::PlayerSet_t
game::spec::ShipList::getPlayersThatCan(int basicFunctionId,
                                        int hullNr,
                                        const game::config::HostConfiguration& config,
                                        ExperienceLevelSet_t levelLimit) const
{
    // ex GHull::getPlayersThatCan
    if (Hull* hull = hulls().get(hullNr)) {
        return hull->getHullFunctions(true).getPlayersThatCan(basicFunctionId, modifiedHullFunctions(), basicHullFunctions(), config, *hull, levelLimit, true)
            |             racialAbilities().getPlayersThatCan(basicFunctionId, modifiedHullFunctions(), basicHullFunctions(), config, *hull, levelLimit, false);
    } else {
        return PlayerSet_t();
    }
}
