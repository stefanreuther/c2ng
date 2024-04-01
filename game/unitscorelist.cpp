/**
  *  \file game/unitscorelist.cpp
  *  \brief Class game::UnitScoreList
  */

#include "game/unitscorelist.hpp"
#include "game/unitscoredefinitionlist.hpp"

// Constructor.
game::UnitScoreList::UnitScoreList()
    : m_items()
{ }

// Destructor.
game::UnitScoreList::~UnitScoreList()
{ }

// Set score value.
void
game::UnitScoreList::set(Index_t index, int16_t value, int16_t turn)
{
    // ex GUnitScores::setScore
    static const Item NULL_ITEM = { 0, -1 };
    if (m_items.size() <= index) {
        m_items.reserve(index+1);
        while (m_items.size() <= index) {
            m_items.push_back(NULL_ITEM);
        }
    }
    m_items[index].turn  = turn;
    m_items[index].value = value;
}

// Merge score value.
void
game::UnitScoreList::merge(Index_t index, int16_t value, int16_t turn)
{
    // ex GUnitScores::mergeScore, phost.pas:AddUnitScoreEntry
    int16_t origValue = 0, origTurn = 0;
    if (!get(index, origValue, origTurn) || origTurn <= turn) {
        set(index, value, turn);
    }
}

// Get score value.
bool
game::UnitScoreList::get(Index_t index, int16_t& value, int16_t& turn) const
{
    // ex GUnitScores::getTurn, GUnitScores::getScore, phost.pas:GetUnitScoreEntry
    if (index < m_items.size() && m_items[index].turn != 0) {
        value = m_items[index].value;
        turn  = m_items[index].turn;
        return true;
    } else {
        return false;
    }
}

// Get score, given a Id.
game::NegativeProperty_t
game::UnitScoreList::getScoreById(int16_t id, const UnitScoreDefinitionList& defs) const
{
    // ex phost.pas:GetExperienceLevel (sort-of)
    Index_t idx;
    int16_t value, turn;
    if (defs.lookup(id, idx) && get(idx, value, turn)) {
        return value;
    } else {
        return afl::base::Nothing;
    }
}
