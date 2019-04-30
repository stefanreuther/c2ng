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
      m_hullAssignments(),
      m_componentNamer(),
      m_friendlyCodes(),
      m_missions()
{ }

// Destructor.
game::spec::ShipList::~ShipList()
{ }

// Get beams.
game::spec::ComponentVector<game::spec::Beam>&
game::spec::ShipList::beams()
{
    return m_beams;
}

// Get beams.
const game::spec::ComponentVector<game::spec::Beam>&
game::spec::ShipList::beams() const
{
    return m_beams;
}

// Get engines.
game::spec::ComponentVector<game::spec::Engine>&
game::spec::ShipList::engines()
{
    return m_engines;
}

// Get engines.
const game::spec::ComponentVector<game::spec::Engine>&
game::spec::ShipList::engines() const
{
    return m_engines;
}

// Get torpedo launchers.
game::spec::ComponentVector<game::spec::TorpedoLauncher>&
game::spec::ShipList::launchers()
{
    return m_launchers;
}

// Get torpedo launchers.
const game::spec::ComponentVector<game::spec::TorpedoLauncher>&
game::spec::ShipList::launchers() const
{
    return m_launchers;
}

// Get hulls.
game::spec::ComponentVector<game::spec::Hull>&
game::spec::ShipList::hulls()
{
    return m_hulls;
}

// Get hulls.
const game::spec::ComponentVector<game::spec::Hull>&
game::spec::ShipList::hulls() const
{
    return m_hulls;
}

// Get basic hull function definitions.
game::spec::BasicHullFunctionList&
game::spec::ShipList::basicHullFunctions()
{
    return m_basicHullFunctions;
}

// Get basic hull function definitions.
const game::spec::BasicHullFunctionList&
game::spec::ShipList::basicHullFunctions() const
{
    return m_basicHullFunctions;
}

// Get modified hull function definitions.
game::spec::ModifiedHullFunctionList&
game::spec::ShipList::modifiedHullFunctions()
{
    return m_modifiedHullFunctions;
}

// Get modified hull function definitions.
const game::spec::ModifiedHullFunctionList&
game::spec::ShipList::modifiedHullFunctions() const
{
    return m_modifiedHullFunctions;
}

// Get racial abilities.
game::spec::HullFunctionAssignmentList&
game::spec::ShipList::racialAbilities()
{
    return m_racialAbilities;
}

// Get racial abilities.
const game::spec::HullFunctionAssignmentList&
game::spec::ShipList::racialAbilities() const
{
    return m_racialAbilities;
}

// Get hull function assignments.
game::spec::HullAssignmentList&
game::spec::ShipList::hullAssignments()
{
    return m_hullAssignments;
}

// Get hull function assignments.
const game::spec::HullAssignmentList&
game::spec::ShipList::hullAssignments() const
{
    return m_hullAssignments;
}

// Get component namer.
game::spec::StandardComponentNameProvider&
game::spec::ShipList::componentNamer()
{
    return m_componentNamer;
}

// Get component namer.
const game::spec::StandardComponentNameProvider&
game::spec::ShipList::componentNamer() const
{
    return m_componentNamer;
}

// Get friendly codes.
game::spec::FriendlyCodeList&
game::spec::ShipList::friendlyCodes()
{
    return m_friendlyCodes;
}

// Get friendly codes.
const game::spec::FriendlyCodeList&
game::spec::ShipList::friendlyCodes() const
{
    return m_friendlyCodes;
}

// Get missions.
game::spec::MissionList&
game::spec::ShipList::missions()
{
    return m_missions;
}

// Get missions.
const game::spec::MissionList&
game::spec::ShipList::missions() const
{
    return m_missions;
}

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
     case Reference::Storm:
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

// Find racial abilities.
void
game::spec::ShipList::findRacialAbilities(const game::config::HostConfiguration& config)
{
    // ex GHull::findRacialAbilities
    // Sanity check
    Hull* referenceHull = hulls().findNext(0);
    if (!referenceHull) {
        return;
    }
    HullFunctionAssignmentList& referenceAssignments = referenceHull->getHullFunctions(true /* assigned to hull */);
    for (size_t i = 0, n = referenceAssignments.getNumEntries(); i < n; ++i) {
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
                for (int hullNr = 1, numHulls = hulls().size(); hullNr < numHulls; ++hullNr) {
                    if (Hull* hull = hulls().get(hullNr)) {
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

// FIXME: remove
// /** Check for special function.
//     \param basic_function basic function, hf_XXX.
//     \param player         check one player only
//     \param hull           true: query hull-specific abilities; Every ship of this
//                           type <em>owned</em> by the player has the function.
//                           false: query ship-specific ability. Every ship of this
//                           type <em>built</em> by the player will have the function.
//     \param levels         experience level restriction. Check only for a function
//                           working at specified levels.
//     \returns true iff player can use the function */
// bool
// GHull::canDoSpecial(int basic_function, int player, bool hull, GExpLevelSet levels) const
// {
//     return getPlayersThatCan(basic_function, hull, levels).contains(player);
// }

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
