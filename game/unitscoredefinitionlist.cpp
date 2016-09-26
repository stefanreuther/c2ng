/**
  *  \file game/unitscoredefinitionlist.cpp
  *
  *  Original (PCC2) comment:
  *
  *  This manages per-unit scores; in particular, those are used for
  *  unit experience in PHost.
  *
  *  Although unit scores are generally stored indexed by type, then
  *  unit (i.e. a global list of score types, associated with a list
  *  of units' scores), we separate these two: a GUnitScoreDefinitions
  *  object defines all scores, and each object contains a list of
  *  applicable stores indexed by the GUnitScoreDefinitions, similar
  *  to interpreter properties split into IntVariableNames for
  *  indexing, and IntDataSegment with the actual content. This
  *  requires us to split up stuff we load, and gather it up again
  *  when we save it, but it allows us to easily clone an object with
  *  score and assign it a new score, either for loading past chart.cc
  *  files or for performing host updates.
  *
  *  It also needs a more memory (i.e. 1500 vector<Item>, many empty,
  *  instead of one or two definitions with one vector<Item> each),
  *  but this is not so much an issue today as it was in PCC 1.x.
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
    // ex GUnitScoreDefinitions::addScoreDefinition
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
    // ex GUnitScoreDefinitions::lookupScore
    for (Index_t i = 0; i < m_definitions.size(); ++i) {
        if (m_definitions[i].id == id) {
            index = i;
            return true;
        }
    }
    return false;
}
