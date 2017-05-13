/**
  *  \file game/score/turnscorelist.cpp
  *  \brief Class game::score::TurnScoreList
  */

#include <cassert>
#include "game/score/turnscorelist.hpp"

// Constructor.
game::score::TurnScoreList::TurnScoreList()
    : m_slotMapping(),
      m_scoreDescriptions(),
      m_fileUsedFutureFeatures(false),
      m_turnScores()
{
    clear();
}

// Destructor.
game::score::TurnScoreList::~TurnScoreList()
{ }

// Reset content.
void
game::score::TurnScoreList::clear()
{
    // ex GStatFile::clear(), GStatFile::init()
    // Clear everything
    std::vector<ScoreId_t>().swap(m_slotMapping);
    std::vector<Description>().swap(m_scoreDescriptions);
    afl::container::PtrVector<TurnScore>().swap(m_turnScores);

    // Set up standard schema
    m_slotMapping.reserve(5);
    m_slotMapping.push_back(ScoreId_Planets);
    m_slotMapping.push_back(ScoreId_Capital);
    m_slotMapping.push_back(ScoreId_Freighters);
    m_slotMapping.push_back(ScoreId_Bases);
    m_slotMapping.push_back(ScoreId_BuildPoints);

    m_fileUsedFutureFeatures = false;
}

// Add a score type.
game::score::TurnScoreList::Slot_t
game::score::TurnScoreList::addSlot(ScoreId_t id)
{
    // ex GStatFile::getAddIndexForScore
    Slot_t result;
    if (!getSlot(id, result)) {
        result = m_slotMapping.size();
        m_slotMapping.push_back(id);
    }
    return result;
}

// Get a score slot by type.
bool
game::score::TurnScoreList::getSlot(ScoreId_t id, Slot_t& out) const
{
    // ex GStatFile::getIndexForScore
    for (size_t i = 0, n = m_slotMapping.size(); i < n; ++i) {
        if (m_slotMapping[i] == id) {
            out = i;
            return true;
        }
    }
    return false;
}

// Add a score description.
bool
game::score::TurnScoreList::addDescription(const Description& d)
{
    // ex GStatFile::addScoreDescription
    for (size_t i = 0, n = m_scoreDescriptions.size(); i < n; ++i) {
        Description& existing = m_scoreDescriptions[i];
        if (existing.scoreId == d.scoreId) {
            // Found it
            if (existing.turnLimit == d.turnLimit
                && existing.winLimit == d.winLimit
                && existing.name == d.name)
            {
                // No change
                return false;
            } else {
                // Update
                existing = d;
                return true;
            }
        }
    }
    m_scoreDescriptions.push_back(d);
    return true;
}

// Get a score description.
const game::score::TurnScoreList::Description*
game::score::TurnScoreList::getDescription(ScoreId_t id) const
{
    // ex GStatFile::getDescriptionForScoreId
    for (size_t i = 0, n = m_scoreDescriptions.size(); i < n; ++i) {
        if (m_scoreDescriptions[i].scoreId == id) {
            return &m_scoreDescriptions[i];
        }
    }
    return 0;
}

// Add a turn.
game::score::TurnScore&
game::score::TurnScoreList::addTurn(int turnNr, const Timestamp& time)
{
    // ex GStatFile::getRecordForTurn
    Index_t index = m_turnScores.size();
    while (index > 0 && turnNr < m_turnScores[index-1]->getTurnNumber()) {
        --index;
    }

    /* Three cases:
       - index = 0 {and turn < [index].turn}
       - turn  = [index-1].turn
       - turn  > [index-1].turn {and turn < [index].turn} */
    if (index > 0 && m_turnScores[index-1]->getTurnNumber() == turnNr) {
        /* If the timestamp differs from the one we know, this is probably
           the result of a re-host. Discard the current scores and start
           anew. */
        if (time != m_turnScores[index-1]->getTimestamp()) {
            m_turnScores.replaceElementNew(index-1, new TurnScore(turnNr, time));
        }
        return *m_turnScores[index-1];
    }

    /* Only cases 1 and 3 remain. We will make a new record and store that
       at [index]. Shift everything to the right first to make room.
       For simplicity, we append the new record at the end, and shift it
       back; this saves quite some exception hassle. */
    m_turnScores.pushBackNew(new TurnScore(turnNr, time));
    if (m_turnScores.size() > 1) {
        for (Index_t i = m_turnScores.size()-1; i > index; --i) {
            /* The last operation this will perform is swapping index-1 and index */
            m_turnScores.swapElements(i-1, i);
        }
    }

    assert(m_turnScores[index]->getTurnNumber() == turnNr);
    return *m_turnScores[index];
}

// Get a turn.
const game::score::TurnScore*
game::score::TurnScoreList::getTurn(int turnNr) const
{
    // FIXME: use binary intersection?
    for (Index_t i = m_turnScores.size(); i > 0; --i) {
        if (m_turnScores[i-1]->getTurnNumber() == turnNr) {
            return m_turnScores[i-1];
        }
    }
    return 0;
}

// Get number of turns stored.
size_t
game::score::TurnScoreList::getNumTurns() const
{
    return m_turnScores.size();
}

// Get turn by index.
const game::score::TurnScore*
game::score::TurnScoreList::getTurnByIndex(size_t index) const
{
    if (index < m_turnScores.size()) {
        return m_turnScores[index];
    } else {
        return 0;
    }
}

// Get number of descriptions stored.
size_t
game::score::TurnScoreList::getNumDescriptions() const
{
    return m_scoreDescriptions.size();
}

// Get description by index.
const game::score::TurnScoreList::Description*
game::score::TurnScoreList::getDescriptionByIndex(size_t index) const
{
    if (index < m_scoreDescriptions.size()) {
        return &m_scoreDescriptions[index];
    } else {
        return 0;
    }
}

// Get number of score types stored.
size_t
game::score::TurnScoreList::getNumScores() const
{
    return m_slotMapping.size();
}

// Get score Id by index.
bool
game::score::TurnScoreList::getScoreByIndex(size_t index, ScoreId_t& result) const
{
    if (index < m_slotMapping.size()) {
        result = m_slotMapping[index];
        return true;
    } else {
        return false;
    }
}

// Set "future features" flag.
void
game::score::TurnScoreList::setFutureFeatures(bool flag)
{
    m_fileUsedFutureFeatures = flag;
}

// Get "future features" flag.
bool
game::score::TurnScoreList::hasFutureFeatures() const
{
    // ex GStatFile::canSaveSafely (sort-of)
    return m_fileUsedFutureFeatures;
}

