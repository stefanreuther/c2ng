/**
  *  \file game/score/turnscorelist.cpp
  *  \brief Class game::score::TurnScoreList
  */

#include <cassert>
#include "game/score/turnscorelist.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/format.hpp"

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

// Add parsed information.
void
game::score::TurnScoreList::addMessageInformation(const game::parser::MessageInformation& info, const Timestamp& ts)
{
    namespace gp = game::parser;
    // ex GStatFile::addMessageInformation
    // info contains:
    // - turn number
    // - optional Id
    // - optional MessageStringValue_t: ms_Name
    // - optional MessageIntegerValue_t: mi_ScoreWinLimit, mi_ScoreTurnLimit
    // - optional MessageScoreValue_t's

    // Pass 1: figure out score description
    ScoreId_t scoreId = static_cast<ScoreId_t>(info.getObjectId());
    afl::base::Optional<int16_t> scoreTurnLimit;
    afl::base::Optional<int32_t> scoreWinLimit;
    afl::base::Optional<String_t> scoreName;

    for (gp::MessageInformation::Iterator_t i = info.begin(); i != info.end(); ++i) {
        if (gp::MessageIntegerValue_t* iv = dynamic_cast<gp::MessageIntegerValue_t*>(*i)) {
            if (iv->getIndex() == gp::mi_ScoreWinLimit) {
                scoreWinLimit = iv->getValue();
            }
            if (iv->getIndex() == gp::mi_ScoreTurnLimit) {
                scoreTurnLimit = static_cast<int16_t>(iv->getValue());
            }
        } else if (gp::MessageStringValue_t* sv = dynamic_cast<gp::MessageStringValue_t*>(*i)) {
            if (sv->getIndex() == gp::ms_Name) {
                scoreName = sv->getValue();
            }
        } else {
            // ignore
        }
    }

    // Find existing score description
    const Description* exist = 0;
    for (size_t i = 0, n = m_scoreDescriptions.size(); i < n; ++i) {
        if (scoreId != 0 && m_scoreDescriptions[i].scoreId == scoreId) {
            exist = &m_scoreDescriptions[i];
            break;
        }
        if (scoreName.isValid() && *scoreName.get() == m_scoreDescriptions[i].name) {
            exist = &m_scoreDescriptions[i];
            break;
        }
    }

    // If we have no description, make one
    Description desc;
    if (exist == 0) {
        if (scoreId != 0) {
            // We have an identifier; check for a name
            // @change PCC2 would translate "Score #%d", we don't.
            // For one, we don't have a translator; for another, this means the name-based matching will work across language changes.
            // FIXME: how about well-known scores that don't need a description, namely: ScoreId_BuildPoints?
            desc.name = scoreName.orElse(afl::string::Format("Score #%d", scoreId));
            desc.scoreId = scoreId;
        } else if (const String_t* name = scoreName.get()) {
            // We have a name. Allocate an Id.
            ScoreId_t i = 1000;
            while (getDescription(i) != 0) {
                ++i;
            }
            desc.name = *name;
            desc.scoreId = i;
        } else {
            // Nothing usable, cannot process this
            return;
        }

        desc.turnLimit = scoreTurnLimit.orElse(-1);
        desc.winLimit  = scoreWinLimit.orElse(-1);
    } else {
        // We have a description, so fill in the blanks
        desc.name      = scoreName.orElse(exist->name);
        desc.scoreId   = exist->scoreId;
        desc.turnLimit = scoreTurnLimit.orElse(exist->turnLimit);
        desc.winLimit  = scoreWinLimit.orElse(exist->winLimit);
    }

    // Now we can add it. This will overwrite a possibly existing definition with our new, updated one.
    addDescription(desc);

    // Pass 2: fill in the scores
    Slot_t si = addSlot(desc.scoreId);
    TurnScore& rec = addTurn(info.getTurnNumber(), ts);
    for (gp::MessageInformation::Iterator_t i = info.begin(); i != info.end(); ++i) {
        if (gp::MessageScoreValue_t* scv = dynamic_cast<gp::MessageScoreValue_t*>(*i)) {
            rec.set(si, scv->getIndex(), scv->getValue());
        }
    }
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

