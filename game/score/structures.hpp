/**
  *  \file game/score/structures.hpp
  *  \brief Structures for Score Database
  */
#ifndef C2NG_GAME_SCORE_STRUCTURES_HPP
#define C2NG_GAME_SCORE_STRUCTURES_HPP

#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "game/v3/structures.hpp"

namespace game { namespace score { namespace structures {

    typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;
    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;
    typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;
    typedef afl::bits::Value<afl::bits::Int32LE> Int32_t;
    typedef afl::bits::Value<afl::bits::FixedString<50> > String50_t;

    using game::v3::structures::GenScore;
    using game::v3::structures::NUM_PLAYERS;

    /// score.cc (PCC2 score db) header.
    struct ScoreHeader {
        uint8_t     signature[8];                               ///< "CCstat0",26.
        UInt32_t    headerSize;                                 ///< Total size of header, pointer to entries.
        UInt16_t    numHeaderFields;                            ///< Fields in this header.
        UInt16_t    numEntries;                                 ///< Number of entries.
        UInt16_t    recordHeaderSize;                           ///< Size of record header.
        UInt16_t    numRecordFields;                            ///< Fields in a record.
        UInt16_t    headerFieldAddress[2];                      ///< Pointers to sub-fields.
    };
    static_assert(sizeof(ScoreHeader) == 24, "sizeof ScoreHeader");

    /// score.cc (PCC2 score db) record header.
    struct ScoreRecordHeader {
        Int16_t     turn;                                       ///< Turn number.
        uint8_t     timestamp[18];                              ///< Time stamp.
    };
    static_assert(sizeof(ScoreRecordHeader) == 20, "sizeof ScoreRecordHeader");

    /// score.cc (PCC2 score db) score description.
    struct ScoreDescription {
        String50_t  name;                                       ///< Name of score. Identifies the score to humans.
        Int16_t     scoreId;                                    ///< Type of score. Identifies the score to programs.
        Int16_t     turnLimit;                                  ///< Turns to keep win limit.
        Int32_t     winLimit;                                   ///< Win limit. If somebody exceeds this limit for turn_limit turns, he wins. -1=no such limit.
    };
    static_assert(sizeof(ScoreDescription) == 58, "sizeof ScoreDescription");

    /// stat.cc (PCC1 score db) header.
    struct StatHeader {
        uint8_t     signature[8];                               ///< "CC-Stat",26.
        Int16_t     numEntries;
        Int16_t     recordSize;
    };
    static_assert(sizeof(StatHeader) == 12, "sizeof StatHeader");

    /// stat.cc (PCC1 score db) entry.
    struct StatRecord {
        ScoreRecordHeader header;                               ///< Record header.
        GenScore    scores[NUM_PLAYERS];                        ///< Player scores.
        Int16_t     pbps[NUM_PLAYERS];                          ///< Build points.
    };
    static_assert(sizeof(StatRecord) == 130, "sizeof StatRecord");

} } }

#endif
