/**
  *  \file game/spec/hullfunctionassignmentlist.cpp
  */

#include "game/spec/hullfunctionassignmentlist.hpp"
#include "game/spec/hullfunction.hpp"
#include "game/spec/basichullfunctionlist.hpp"

game::spec::HullFunctionAssignmentList::HullFunctionAssignmentList()
    : m_entries()
{
    clear();
}

game::spec::HullFunctionAssignmentList::~HullFunctionAssignmentList()
{ }

void
game::spec::HullFunctionAssignmentList::clear()
{
    // ex GHull::clearSpecialFunctions
    m_entries.clear();

//     /* FIXME comment: Some functions can be modified by config options. Those are described
//        by GHullFunctionData::getDefaultAssignment(). We add an entry effectively
//        saying "no change", to make enumerateHullFunctions() query
//        getDefaultAssignment(). The alternative would be to special-case it.
//        Note that this uses the fact that a hf_XXX can be used as a dev_t.
//        The converse does not hold. */
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(HullFunction::Tow),               PlayerSet_t(), PlayerSet_t()));
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(HullFunction::Boarding),          PlayerSet_t(), PlayerSet_t()));
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(HullFunction::AntiCloakImmunity), PlayerSet_t(), PlayerSet_t()));
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(HullFunction::PlanetImmunity),    PlayerSet_t(), PlayerSet_t()));
    m_entries.push_back(Entry(ModifiedHullFunctionList::Function_t(HullFunction::FullWeaponry),      PlayerSet_t(), PlayerSet_t()));
}

size_t
game::spec::HullFunctionAssignmentList::getNumEntries() const
{
    return m_entries.size();
}


// /** Modify hull function assignment.
//     \param id               Internal (modified) function Id, comes from
//                             GHullFunctionData::getIdFromFunction etc.
//     \param add              Allow these players to use it
//     \param remove           Then disallow these players using it
//     \param assign_to_hull   true: this function is available to all ships of this type
//                             false: this function is assigned to newly-built ships */
void
game::spec::HullFunctionAssignmentList::change(ModifiedHullFunctionList::Function_t function, PlayerSet_t add, PlayerSet_t remove)
{
    // ex GHull::changeSpecialFunction
    if (Entry* p = find(function)) {
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
        m_entries.push_back(Entry(function, add-remove, remove-add));
    } else {
        // Empty addition and function not found. This is a no-op (removing from empty element).
    }
}

game::spec::HullFunctionAssignmentList::Entry*
game::spec::HullFunctionAssignmentList::find(ModifiedHullFunctionList::Function_t function)
{
    // ex GHull::getSpecialFunctionPtr
    for (size_t i = 0, n = m_entries.size(); i < n; ++i) {
        if (m_entries[i].m_function == function) {
            return &m_entries[i];
        }
    }
    return 0;
}

void
game::spec::HullFunctionAssignmentList::remove(ModifiedHullFunctionList::Function_t function)
{
    for (size_t i = 0, n = m_entries.size(); i < n; ++i) {
        if (m_entries[i].m_function == function) {
            m_entries.erase(m_entries.begin() + i);
            break;
        }
    }
}

game::spec::HullFunctionAssignmentList::Entry*
game::spec::HullFunctionAssignmentList::get(size_t i)
{
    if (i < m_entries.size()) {
        return &m_entries[i];
    } else {
        return 0;
    }
}

void
game::spec::HullFunctionAssignmentList::getAll(HullFunctionList& out,
                                               const ModifiedHullFunctionList& definitions,
                                               const game::config::HostConfiguration& config,
                                               const Hull& hull,
                                               PlayerSet_t playerLimit,
                                               ExperienceLevelSet_t levelLimit,
                                               HullFunction::Kind kind)
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
    PlayerSet_t result;
    for (size_t i = 0, n = m_entries.size(); i < n; ++i) {
        PlayerSet_t players;
        if (useDefaults) {
            players += HullFunction::getDefaultAssignment(int32_t(m_entries[i].m_function), config, hull);
        }
        players += m_entries[i].m_addedPlayers;
        players -= m_entries[i].m_removedPlayers;
        if (!players.empty()) {
            HullFunction function;
            if (definitions.getFunctionDefinition(m_entries[i].m_function, function)) {
                if (function.getLevels().containsAnyOf(levelLimit)) {
                    if (basicDefinitions.matchFunction(basicFunctionId, function.getBasicFunctionId())) {
                        result += players;
                    }
                }
            }
        }
    }
    return result;
}
