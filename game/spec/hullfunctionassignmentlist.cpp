/**
  *  \file game/spec/hullfunctionassignmentlist.cpp
  *  \brief Class game::spec::HullFunctionAssignmentList
  */

#include "game/spec/hullfunctionassignmentlist.hpp"
#include "game/spec/hullfunction.hpp"
#include "game/spec/basichullfunctionlist.hpp"

// Constructor.
game::spec::HullFunctionAssignmentList::HullFunctionAssignmentList()
    : m_entries()
{
    clear();
}

// Destructor.
game::spec::HullFunctionAssignmentList::~HullFunctionAssignmentList()
{ }

// Clear.
void
game::spec::HullFunctionAssignmentList::clear()
{
    // ex GHull::clearSpecialFunctions
    m_entries.clear();

    // Some functions can be modified by config options.
    // Those are described HullFunction::getDefaultAssignment().
    // We add an entry effectively saying "no change", to make getAll() query getDefaultAssignment().
    // The alternative would be to special-case it.
    // Note that this uses the fact that a HullFunction constant can be used as a Function_t.
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(BasicHullFunction::Tow),               PlayerSet_t(), PlayerSet_t()));
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(BasicHullFunction::Boarding),          PlayerSet_t(), PlayerSet_t()));
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(BasicHullFunction::AntiCloakImmunity), PlayerSet_t(), PlayerSet_t()));
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(BasicHullFunction::PlanetImmunity),    PlayerSet_t(), PlayerSet_t()));
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(BasicHullFunction::FullWeaponry),      PlayerSet_t(), PlayerSet_t()));
}

// Get number of entries.
size_t
game::spec::HullFunctionAssignmentList::getNumEntries() const
{
    return m_entries.size();
}


// Modify hull function assignment.
void
game::spec::HullFunctionAssignmentList::change(ModifiedHullFunctionList::Function_t function, PlayerSet_t add, PlayerSet_t remove)
{
    // ex GHull::changeSpecialFunction
    // This function is defines as "add, then remove". Make add/remove disjoint.
    add -= remove;
    if (Entry* p = const_cast<Entry*>(findEntry(function))) {
        // Found the function: modify its attributes.
        p->m_addedPlayers += add;
        p->m_addedPlayers -= remove;
        p->m_removedPlayers += remove;
        p->m_removedPlayers -= add;
    } else if (!add.empty()) {
        // Function not found.
        // Remove-only settings are only relevant for functions that have a variable default.
        // Since all to which this applies are already on the list (see clear()),
        // they will always hit the case above and we need not make a new entry for those.
        m_entries.push_back(Entry(function, add, remove));
    } else {
        // Empty addition and function not found. This is a no-op (removing from empty element).
    }
}

// Find entry, given a function Id.
const game::spec::HullFunctionAssignmentList::Entry*
game::spec::HullFunctionAssignmentList::findEntry(ModifiedHullFunctionList::Function_t function) const
{
    // ex GHull::getSpecialFunctionPtr
    for (size_t i = 0, n = m_entries.size(); i < n; ++i) {
        if (m_entries[i].m_function == function) {
            return &m_entries[i];
        }
    }
    return 0;
}

// Remove entry, given a function Id.
void
game::spec::HullFunctionAssignmentList::removeEntry(ModifiedHullFunctionList::Function_t function)
{
    for (size_t i = 0, n = m_entries.size(); i < n; ++i) {
        if (m_entries[i].m_function == function) {
            m_entries.erase(m_entries.begin() + i);
            break;
        }
    }
}

// Get entry, given an index.
const game::spec::HullFunctionAssignmentList::Entry*
game::spec::HullFunctionAssignmentList::getEntryByIndex(size_t i) const
{
    if (i < m_entries.size()) {
        return &m_entries[i];
    } else {
        return 0;
    }
}

// Get all effective assignments as a HullFunctionList.
void
game::spec::HullFunctionAssignmentList::getAll(HullFunctionList& out,
                                               const ModifiedHullFunctionList& definitions,
                                               const game::config::HostConfiguration& config,
                                               const Hull& hull,
                                               PlayerSet_t playerLimit,
                                               ExperienceLevelSet_t levelLimit,
                                               HullFunction::Kind kind) const
{
    // ex GHull::enumerateSet
    for (size_t i = 0, n = m_entries.size(); i < n; ++i) {
        PlayerSet_t players;
        if (kind == HullFunction::AssignedToHull) {
            players += HullFunction::getDefaultAssignment(int32_t(m_entries[i].m_function), config, hull);
        }
        players += m_entries[i].m_addedPlayers;
        players -= m_entries[i].m_removedPlayers;
        players &= playerLimit;
        if (!players.empty()) {
            HullFunction function;
            if (definitions.getFunctionDefinition(m_entries[i].m_function, function)) {
                if (function.getLevels().containsAnyOf(levelLimit)) {
                    function.setPlayers(players);
                    function.setKind(kind);
                    out.add(function);
                }
            }
        }
    }
}

// Get players that can perform a particular basic function.
game::PlayerSet_t
game::spec::HullFunctionAssignmentList::getPlayersThatCan(int basicFunctionId,
                                                          const ModifiedHullFunctionList& definitions,
                                                          const BasicHullFunctionList& basicDefinitions,
                                                          const game::config::HostConfiguration& config,
                                                          const Hull& hull,
                                                          ExperienceLevelSet_t levelLimit,
                                                          bool useDefaults) const
{
    // ex GHull::checkSet
    /*
     *  This used to first determine the 'players' set and resolve that into a hull function only if that was nonempty.
     *  It turns out that determining the players is the expensive part (host configuration access).
     *  Reversing the tests brings down the time for the MovementPredictor test from 15->2.5 seconds.
     */
    PlayerSet_t result;
    for (size_t i = 0, n = m_entries.size(); i < n; ++i) {
        HullFunction function;
        if (definitions.getFunctionDefinition(m_entries[i].m_function, function)) {
            if (function.getLevels().containsAnyOf(levelLimit)) {
                if (basicDefinitions.matchFunction(basicFunctionId, function.getBasicFunctionId())) {
                    PlayerSet_t players;
                    if (useDefaults) {
                        players += HullFunction::getDefaultAssignment(int32_t(m_entries[i].m_function), config, hull);
                    }
                    players += m_entries[i].m_addedPlayers;
                    players -= m_entries[i].m_removedPlayers;
                    result += players;
                }
            }
        }
    }
    return result;
}
