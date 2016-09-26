/**
  *  \file game/unitscorelist.cpp
  */

#include "game/unitscorelist.hpp"

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
    // ex GUnitScores::mergeScore
    int16_t origValue = 0, origTurn = 0;
    if (!get(index, origValue, origTurn) || origTurn <= turn) {
        set(index, value, turn);
    }
}


bool
game::UnitScoreList::get(Index_t index, int16_t& value, int16_t& turn) const
{
    // ex GUnitScores::getTurn, GUnitScores::getScore
    if (index < m_items.size() && m_items[index].turn != 0) {
        value = m_items[index].value;
        turn  = m_items[index].turn;
        return true;
    } else {
        return false;
    }
}
