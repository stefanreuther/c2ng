/**
  *  \file game/score/loader.cpp
  */

#include <cstring>
#include "game/score/loader.hpp"
#include "game/score/structures.hpp"
#include "afl/except/fileformatexception.hpp"
#include "game/score/turnscorelist.hpp"
#include "afl/base/growablememory.hpp"

namespace structures = game::score::structures;

namespace {
    static const char SCORE_FILE_SIG[] = { 'C', 'C', 's', 't', 'a', 't', '0', 26 };

    void loadRecord(afl::io::Stream& in,
                    game::score::TurnScore& record,
                    const std::vector<game::score::TurnScore::Slot_t>& slots)
    {
        // ex GStatRecord::loadData (sort-of)
        for (size_t slotIndex = 0, numSlots = slots.size(); slotIndex < numSlots; ++slotIndex) {
            structures::Int32_t row[structures::NUM_PLAYERS];
            in.fullRead(afl::base::fromObject(row));
            for (size_t i = 0; i < structures::NUM_PLAYERS; ++i) {
                int32_t value = row[i];
                if (value != -1) {
                    record.set(slots[slotIndex], i+1, value);
                }
            }
        }
    }
}

game::score::Loader::Loader(afl::string::Translator& tx, afl::charset::Charset& cs)
    : m_translator(tx),
      m_charset(cs)
{ }

void
game::score::Loader::load(TurnScoreList& list, afl::io::Stream& in)
{
    // ex GStatFile::load
    list.clear();

    // Head header
    structures::ScoreHeader new_header;
    in.fullRead(afl::base::fromObject(new_header));
    if (std::memcmp(new_header.signature, SCORE_FILE_SIG, sizeof(SCORE_FILE_SIG)) != 0) {
        throw afl::except::FileProblemException(in, m_translator.translateString("File is missing required signature"));
    }
    if (new_header.headerSize < sizeof(new_header)
        || new_header.numHeaderFields < 2
        || new_header.recordHeaderSize < sizeof(structures::ScoreRecordHeader)
        || new_header.headerFieldAddress[0] < sizeof(new_header)
        || new_header.headerFieldAddress[1] < sizeof(new_header))
    {
        throw afl::except::FileFormatException(in, m_translator.translateString("Unsupported file format"));
    }

    // FIXME: port this
    //     if (new_header.numHeaderFields > 2 || new_header.recordHeaderSize > TScoreRecordHeader::size)
    //         file_used_future_features = true;

    /* Read record description */
    in.setPos(new_header.headerFieldAddress[0]);
    afl::base::GrowableMemory<structures::Int16_t> slotIds;
    slotIds.resize(new_header.numRecordFields);
    in.fullRead(slotIds.toBytes());

    // Convert to indexes
    std::vector<TurnScore::Slot_t> slotIndexes;
    for (size_t i = 0; i < slotIds.size(); ++i) {
        slotIndexes.push_back(list.addSlot(*slotIds.at(i)));
    }

    /* Read score descriptions */
    in.setPos(new_header.headerFieldAddress[1]);
    structures::UInt16_t rawNumDesc;
    in.fullRead(rawNumDesc.m_bytes);
    for (size_t i = 0, n = rawNumDesc; i < n; ++i) {
        // Read raw description
        structures::ScoreDescription rawDesc;
        in.fullRead(afl::base::fromObject(rawDesc));

        // Build description
        TurnScoreList::Description cookedDesc = {
            m_charset.decode(afl::string::toMemory(rawDesc.name)),
            rawDesc.scoreId,
            rawDesc.turnLimit,
            rawDesc.winLimit
        };
        list.addDescription(cookedDesc);
    }

    /* Read scores */
    in.setPos(new_header.headerSize);
    // FIXME: loaded_records.reserve(new_header.numEntries);
    for (size_t i = 0, n = new_header.numEntries; i < n; ++i) {
        structures::ScoreRecordHeader rh;
        in.fullRead(afl::base::fromObject(rh));
        if (new_header.recordHeaderSize > sizeof(rh)) {
            in.setPos(in.getPos() + (new_header.recordHeaderSize - sizeof(rh)));
        }

        // Create the record
        TurnScore& r = list.addTurn(rh.turn, rh.timestamp);
        loadRecord(in, r, slotIndexes);
    }
}


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

// FIXME: port
// /** Save to stream. Saves a header and the data. */
// void
// GStatRecord::save(Stream& s) const
// {
//     TScoreRecordHeader header;
//     header.turn = turn;
//     timestamp.storeRawData(header.timestamp);
//     storeStructureT(s, header);
//     for (GStatIndex i = 0; i < scores.size(); ++i) {
//         TInt32 tmp;
//         tmp.value = scores[i];
//         storeStructureT(s, tmp);
//     }
// }
