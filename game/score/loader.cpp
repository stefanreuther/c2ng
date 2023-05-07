/**
  *  \file game/score/loader.cpp
  *  \brief Class game::score::Loader
  */

#include <cstring>
#include "game/score/loader.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/except/fileformatexception.hpp"
#include "game/score/structures.hpp"
#include "game/score/turnscorelist.hpp"

namespace st = game::score::structures;

namespace {
    const uint8_t SCORE_FILE_SIG[] = { 'C', 'C', 's', 't', 'a', 't', '0', 26 };
    const uint8_t STAT_FILE_SIG[]  = { 'C', 'C', '-', 'S', 't', 'a', 't', 26 };

    void loadRecord(afl::io::Stream& in,
                    game::score::TurnScore& record,
                    const std::vector<game::score::TurnScore::Slot_t>& slots)
    {
        // ex GStatRecord::loadData (sort-of)
        for (size_t slotIndex = 0, numSlots = slots.size(); slotIndex < numSlots; ++slotIndex) {
            st::Int32_t row[st::NUM_PLAYERS];
            in.fullRead(afl::base::fromObject(row));
            for (int i = 0; i < st::NUM_PLAYERS; ++i) {
                int32_t value = row[i];
                if (value != -1) {
                    record.set(slots[slotIndex], i+1, value);
                }
            }
        }
    }
}

// Constructor.
game::score::Loader::Loader(afl::string::Translator& tx, afl::charset::Charset& cs)
    : m_translator(tx),
      m_charset(cs)
{ }

// Load PCC2 score file (score.cc).
void
game::score::Loader::load(TurnScoreList& list, afl::io::Stream& in)
{
    // ex GStatFile::load
    list.clear();

    // Head header
    st::ScoreHeader new_header;
    in.fullRead(afl::base::fromObject(new_header));
    if (std::memcmp(new_header.signature, SCORE_FILE_SIG, sizeof(SCORE_FILE_SIG)) != 0) {
        throw afl::except::FileProblemException(in, m_translator.translateString("File is missing required signature"));
    }
    if (new_header.headerSize < sizeof(new_header)
        || new_header.numHeaderFields < 2
        || new_header.recordHeaderSize < sizeof(st::ScoreRecordHeader)
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
    afl::base::GrowableMemory<st::Int16_t> slotIds;
    slotIds.resize(new_header.numRecordFields);
    in.fullRead(slotIds.toBytes());

    // Convert to indexes
    std::vector<TurnScore::Slot_t> slotIndexes;
    for (size_t i = 0; i < slotIds.size(); ++i) {
        slotIndexes.push_back(list.addSlot(*slotIds.at(i)));
    }

    /* Read score descriptions */
    in.setPos(new_header.headerFieldAddress[1]);
    st::UInt16_t rawNumDesc;
    in.fullRead(rawNumDesc.m_bytes);
    for (size_t i = 0, n = rawNumDesc; i < n; ++i) {
        // Read raw description
        st::ScoreDescription rawDesc;
        in.fullRead(afl::base::fromObject(rawDesc));

        // Build description
        TurnScoreList::Description cookedDesc(m_charset.decode(rawDesc.name),
                                              rawDesc.scoreId,
                                              rawDesc.turnLimit,
                                              rawDesc.winLimit);
        list.addDescription(cookedDesc);
    }

    /* Read scores */
    in.setPos(new_header.headerSize);
    // FIXME: loaded_records.reserve(new_header.numEntries);
    for (size_t i = 0, n = new_header.numEntries; i < n; ++i) {
        st::ScoreRecordHeader rh;
        in.fullRead(afl::base::fromObject(rh));
        if (new_header.recordHeaderSize > sizeof(rh)) {
            in.setPos(in.getPos() + (new_header.recordHeaderSize - sizeof(rh)));
        }

        // Create the record
        TurnScore& r = list.addTurn(rh.turn, rh.timestamp);
        loadRecord(in, r, slotIndexes);
    }
}

// Load PCC1 score file (stat.cc).
void
game::score::Loader::loadOldFile(TurnScoreList& list, afl::io::Stream& in)
{
    // ex GStatFile::loadOldFile
    // Start with default schema
    list.clear();

    st::StatHeader h;
    in.fullRead(afl::base::fromObject(h));
    if (std::memcmp(h.signature, STAT_FILE_SIG, sizeof(STAT_FILE_SIG)) != 0
        || h.recordSize < int16_t(sizeof(st::StatRecord))
        || h.numEntries < 0)
    {
        throw afl::except::FileFormatException(in, m_translator.translateString("Unsupported file format"));
    }

    // Figure out slot positions
    const TurnScoreList::Slot_t pi = list.addSlot(ScoreId_Planets);
    const TurnScoreList::Slot_t ci = list.addSlot(ScoreId_Capital);
    const TurnScoreList::Slot_t fi = list.addSlot(ScoreId_Freighters);
    const TurnScoreList::Slot_t bi = list.addSlot(ScoreId_Bases);
    const TurnScoreList::Slot_t qi = list.addSlot(ScoreId_BuildPoints);

    // read individual records
    const int16_t numEntries = h.numEntries;
    const int16_t recordSize = h.recordSize;
    for (int32_t i = 0; i < numEntries; ++i) {
        st::StatRecord r;
        in.fullRead(afl::base::fromObject(r));
        if (recordSize != int16_t(sizeof(r))) {
            in.setPos(in.getPos() + recordSize - sizeof(r));
        }

        TurnScore& sr = list.addTurn(r.header.turn, r.header.timestamp);
        for (int pl = 1; pl <= st::NUM_PLAYERS; ++pl) {
            sr.set(pi, pl, int16_t(r.scores[pl-1].numPlanets));
            sr.set(ci, pl, int16_t(r.scores[pl-1].numCapitalShips));
            sr.set(fi, pl, int16_t(r.scores[pl-1].numFreighters));
            sr.set(bi, pl, int16_t(r.scores[pl-1].numBases));
            sr.set(qi, pl, int16_t(r.pbps[pl-1]));
        }
    }
}


// Save PCC2 score file (score.cc).
void
game::score::Loader::save(const TurnScoreList& list, afl::io::Stream& out)
{
    // ex GStatFile::save(Stream& s)
    const afl::io::Stream::FileSize_t start = out.getPos();

    // Write preliminary header
    st::ScoreHeader header;
    afl::base::fromObject(header).fill(0);
    out.fullWrite(afl::base::fromObject(header));

    // Write section 1: record definitions
    const size_t numScores = list.getNumScores();
    header.headerFieldAddress[0] = uint16_t(out.getPos());
    header.numRecordFields = uint16_t(numScores);
    for (size_t i = 0; i < numScores; ++i) {
        ScoreId_t id = list.getScoreByIndex(i).orElse(0);

        st::UInt16_t packedId;
        packedId = uint16_t(id);
        out.fullWrite(packedId.m_bytes);
    }

    // Write section 2: score definitions
    const size_t numDescriptions = list.getNumDescriptions();
    header.headerFieldAddress[1] = uint16_t(out.getPos());

    st::UInt16_t packedNum;
    packedNum = uint16_t(numDescriptions);
    out.fullWrite(packedNum.m_bytes);
    for (size_t i = 0; i < numDescriptions; ++i) {
        st::ScoreDescription sd;
        afl::base::fromObject(sd).fill(0);
        if (const TurnScoreList::Description* p = list.getDescriptionByIndex(i)) {
            sd.name = m_charset.encode(afl::string::toMemory(p->name));
            sd.scoreId = p->scoreId;
            sd.turnLimit = p->turnLimit;
            sd.winLimit = p->winLimit;
        }
        out.fullWrite(afl::base::fromObject(sd));
    }

    // Write data
    const size_t numTurns = list.getNumTurns();
    header.headerSize = uint16_t(out.getPos());
    header.numHeaderFields = 2;
    header.numEntries = uint16_t(numTurns);
    header.recordHeaderSize = sizeof(st::ScoreRecordHeader);

    for (size_t i = 0; i < numTurns; ++i) {
        if (const TurnScore* p = list.getTurnByIndex(i)) {
            // ex GStatRecord::save
            // - header
            st::ScoreRecordHeader rh;
            rh.turn = int16_t(p->getTurnNumber());
            p->getTimestamp().storeRawData(rh.timestamp);
            out.fullWrite(afl::base::fromObject(rh));

            // - content
            afl::base::GrowableMemory<st::Int32_t> buffer;
            for (size_t slot = 0; slot < numScores; ++slot) {
                for (int player = 1; player <= st::NUM_PLAYERS; ++player) {
                    st::Int32_t value;
                    value = p->get(slot, player).orElse(-1);
                    buffer.append(value);
                }
            }
            out.fullWrite(buffer.toBytes());
        }
    }

    // Write header again
    afl::base::Bytes_t(header.signature).copyFrom(SCORE_FILE_SIG);
    out.setPos(start);
    out.fullWrite(afl::base::fromObject(header));
}
