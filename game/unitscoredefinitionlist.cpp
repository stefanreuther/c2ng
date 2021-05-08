/**
  *  \file game/unitscoredefinitionlist.cpp
  *  \brief Class game::UnitScoreDefinitionList
  */

#include "game/unitscoredefinitionlist.hpp"


// Constructor.
game::UnitScoreDefinitionList::UnitScoreDefinitionList()
    : m_definitions()
{ }

// Destructor.
game::UnitScoreDefinitionList::~UnitScoreDefinitionList()
{ }

// Add a score definition.
game::UnitScoreDefinitionList::Index_t
game::UnitScoreDefinitionList::add(const Definition& def)
{
    // ex GUnitScoreDefinitions::addScoreDefinition, phost.pas:AddUnitScore
    Index_t index;
    if (!lookup(def.id, index)) {
        index = m_definitions.size();
        m_definitions.push_back(def);
    }
    return index;
}

// Get score definition by index.
const game::UnitScoreDefinitionList::Definition*
game::UnitScoreDefinitionList::get(Index_t index) const
{
    // ex GUnitScoreDefinitions::getScoreDefinition
    if (index < m_definitions.size()) {
        return &m_definitions[index];
    } else {
        return 0;
    }
}

// Get number of scores stored.
game::UnitScoreDefinitionList::Index_t
game::UnitScoreDefinitionList::getNumScores() const
{
    // ex GUnitScoreDefinitions::getNumScores
    return m_definitions.size();
}

// Look up score by identifier.
bool
game::UnitScoreDefinitionList::lookup(int16_t id, Index_t& index) const
{
    // ex GUnitScoreDefinitions::lookupScore, phost.pas:GetUnitScore
    for (Index_t i = 0; i < m_definitions.size(); ++i) {
        if (m_definitions[i].id == id) {
            index = i;
            return true;
        }
    }
    return false;
}
