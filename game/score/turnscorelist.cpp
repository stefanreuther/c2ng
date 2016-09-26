/**
  *  \file game/score/turnscorelist.cpp
  */

#include <cassert>
#include "game/score/turnscorelist.hpp"

// /** Constructor.
//     Makes a blank statistics file with a default schema. */
game::score::TurnScoreList::TurnScoreList()
    : m_slotMapping(),
      m_scoreDescriptions(),
      m_turnScores()
{
    clear();
}

game::score::TurnScoreList::~TurnScoreList()
{ }

void
game::score::TurnScoreList::clear()
{
    // ex GStatFile::clear(), GStatFile::init()
    // Clear everything
    std::vector<ScoreId_t>().swap(m_slotMapping);
    std::vector<Description>().swap(m_scoreDescriptions);
    afl::container::PtrVector<TurnScore>().swap(m_turnScores);

    // Set up standard shcema
    m_slotMapping.reserve(5);
    m_slotMapping.push_back(ScoreId_Planets);
    m_slotMapping.push_back(ScoreId_Capital);
    m_slotMapping.push_back(ScoreId_Freighters);
    m_slotMapping.push_back(ScoreId_Bases);
    m_slotMapping.push_back(ScoreId_BuildPoints);
}

// /** Get index for particular score, or add it. If the score does not exist yet,
//     changes the schema to add it.
//     \param score_id Score identifier, ScoreId_xxx.
//     \return Index (>= 0) */
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

// /** Get index for a particular score.
//     \param score_id Score identifier, ScoreId_xxx.
//     \return Index (>= 0) or nil */
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

// /** Add score description. If the description already exists, it is overwritten.
//     \param desc Description to add
//     \return true if this adds or changes a description, false if the new
//     description is identical to an existing one */
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

// /** Get description for a particular score Id.
//     \param score_id Score identifier, ScoreId_xxx
//     \return pointer to description, or null if none stored in file */
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

// /** Get score record for a turn. If none exists, adds one.
//     \param turn Turn number (used as key)
//     \param ts   Timestamp
//     \return reference to record */
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


// /** Constructor.
//     Loads a statistics file. */
// void
// GStatFile::load(Stream& s)
// {
//     clear();

//     char tmp[2];
//     TScoreHeader new_header;

//     /* Try to read file */
//     getStructureT(s, new_header);
//     if (std::memcmp(new_header.signature, SCORE_FILE_SIG, sizeof(new_header.signature)) != 0)
//         throw FileFormatException(s, _("File is missing required signature"));
//     if (new_header.header_size < new_header.size
//         || new_header.header_fields < 2
//         || new_header.record_header < TScoreRecordHeader::size
//         || new_header.subfield[0] < new_header.size
//         || new_header.subfield[1] < new_header.size)
//     {
//         throw FileFormatException(s, _("Unsupported file format"));
//     }

//     if (new_header.header_fields > 2 || new_header.record_header > TScoreRecordHeader::size)
//         file_used_future_features = true;

//     /* Read record description */
//     s.seek(new_header.subfield[0]);
//     for (uint32_t i = 0; i < new_header.record_fields; ++i) {
//         s.readT(tmp, 2);
//         record_defs.push_back(getInt16(tmp));
//     }

//     /* Read score descriptions */
//     s.seek(new_header.subfield[1]);
//     s.readT(tmp, 2);
//     const uint16_t num_descriptions = getUint16(tmp);
//     for (uint32_t i = 0; i < num_descriptions; ++i) {
//         TScoreDescription desc;
//         getStructureT(s, desc);
//         score_descriptions.push_back(desc);
//     }

//     /* Read scores */
//     s.seek(new_header.header_size);
//     loaded_records.reserve(new_header.entries);
//     for (uint32_t i = 0; i < new_header.entries; ++i) {
//         TScoreRecordHeader rh;
//         getStructureT(s, rh);
//         if (new_header.record_header > TScoreRecordHeader::size)
//             s.seek(s.getPos() + (new_header.record_header - TScoreRecordHeader::size));

//         GStatRecord* r = new GStatRecord(rh.turn,
//                                          rh.timestamp,
//                                          new_header.record_fields);
//         loaded_records.push_back(r);
//         r->loadData(s);
//     }
// }

// /** Load old-style (PCC1.x) statistics file. */
// void
// GStatFile::loadOldFile(Stream& s)
// {
//     /* start with default schema */
//     init();

//     TStatHeader h;
//     getStructureT(s, h);
//     if (std::memcmp(h.signature, "CC-Stat\032", sizeof(h.signature)) != 0
//         || h.entry_size < TStatRecord::size
//         || h.entries < 0)
//     {
//         throw FileFormatException(s, _("Unsupported file format"));
//     }

//     /* figure out positions (since we use the default schema, we could also use
//        constants, but it's more flexible this way) */
//     const GStatIndex pi = getAddIndexForScore(ScoreId_Planets);
//     const GStatIndex ci = getAddIndexForScore(ScoreId_Capital);
//     const GStatIndex fi = getAddIndexForScore(ScoreId_Freighters);
//     const GStatIndex bi = getAddIndexForScore(ScoreId_Bases);
//     const GStatIndex qi = getAddIndexForScore(ScoreId_BuildPoints);

//     /* read individual records */
//     for (int32_t i = 0; i < h.entries; ++i) {
//         TStatRecord r;
//         getStructureT(s, r);
//         if (h.entry_size != TStatRecord::size)
//             s.seek(s.getPos() + h.entry_size - TStatRecord::size);

//         GStatRecord& sr = getRecordForTurn(r.header.turn, r.header.timestamp);
//         for (int pl = 1; pl <= NUM_PLAYERS; ++pl) {
//             sr(pi, pl) = r.scores[pl-1].num_planets;
//             sr(ci, pl) = r.scores[pl-1].num_capital;
//             sr(fi, pl) = r.scores[pl-1].num_freighters;
//             sr(bi, pl) = r.scores[pl-1].num_bases;
//             sr(qi, pl) = r.pbps[pl-1];
//         }
//     }
// }


// /** Save file. Writes the complete fileto the specified stream.
//     This will save the file even if canSaveSafely() returns false; in this
//     case, the new copy will contain less information than the file this
//     was loaded from. */
// void
// GStatFile::save(Stream& s)
// {
//     using std::memcpy;
//     char tmp[2];
//     long start = s.getPos();

//     /* Write preliminary header */
//     TScoreHeader header;
//     zeroFill(header);
//     storeStructureT(s, header);

//     /* Write section 1: record definitions */
//     header.subfield[0]   = s.getPos();
//     header.record_fields = record_defs.size();
//     for (uint32_t i = 0; i < record_defs.size(); ++i) {
//         storeInt16(tmp, record_defs[i]);
//         s.writeT(tmp, 2);
//     }

//     /* Write section 2: score definitions */
//     header.subfield[1] = s.getPos();
//     storeInt16(tmp, score_descriptions.size());
//     s.writeT(tmp, 2);
//     for (uint32_t i = 0; i < score_descriptions.size(); ++i)
//         storeStructureT(s, score_descriptions[i]);

//     /* Write data */
//     header.header_size   = s.getPos();
//     header.header_fields = 2;
//     header.entries       = loaded_records.size();
//     header.record_header = TScoreRecordHeader::size;
//     for (uint32_t i = 0; i < loaded_records.size(); ++i)
//         loaded_records[i]->save(s);

//     /* Write header again */
//     memcpy(header.signature, SCORE_FILE_SIG, sizeof(header.signature));
//     s.seek(start);
//     storeStructureT(s, header);
// }
