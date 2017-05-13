/**
  *  \file game/db/structures.hpp
  *  \brief Database File Structures
  */
#ifndef C2NG_GAME_DB_STRUCTURES_HPP
#define C2NG_GAME_DB_STRUCTURES_HPP

#include "afl/base/staticassert.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"

namespace game { namespace db { namespace structures {

    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;
    typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;
    typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;
    typedef afl::bits::Value<afl::bits::FixedString<50> > String50_t;

    struct Header {
        char signature[8];
        UInt16_t turnNumber;
        UInt16_t dataStart;
        UInt16_t numPlanetProperties;
        UInt16_t numShipProperties;
    };
    static_assert(sizeof(Header) == 16, "sizeof Header");

    const char SIGNATURE[8] = {'C','C','c','h','a','r','t',26};


    struct BlockHeader {
        UInt16_t blockType;
        UInt32_t size;
    };
    static_assert(sizeof(BlockHeader) == 6, "sizeof BlockHeader");

    /** Starchart database record numbers. */
    const uint16_t rPlanetHistory  = 1;
    const uint16_t rShipHistory    = 2;
    const uint16_t rShipTrack      = 3;
    const uint16_t rMinefield      = 4;
    const uint16_t rPainting       = 5;
    const uint16_t rAutoBuild      = 6;
    const uint16_t rShipProperty   = 7;
    const uint16_t rPlanetProperty = 8;

    // PCC 1.1.6+:
    const uint16_t rShipScore      = 9;
    const uint16_t rPlanetScore    = 10;
    const uint16_t rPaintingTags   = 11;

    // PCC 1.1.7+:
    const uint16_t rUfoHistory     = 12;


    /// User drawing (rPainting, 5).
    struct DatabaseDrawing {
        uint8_t     type;                                       ///< Painting type, and comment flag.
        uint8_t     color;                                      ///< Painting color.
        Int16_t     x1, y1;                                     ///< Left-top or center position.
        Int16_t     x2, y2;                                     ///< Bottom-right position, radius or shape.
        Int16_t     tag;                                        ///< User-defined tag.
        Int16_t     expirationTurn;                             ///< Turn of expiry.
    };
    static_assert(sizeof(DatabaseDrawing) == 14, "sizeof DatabaseDrawing");

    /// Autobuild settings (rAutoBuild, 6).
    struct AutobuildSettings {
        Int16_t     goal[4];                                    ///< Target number, 0..999; 1000 meaning "max".
        int8_t      speed[4];                                   ///< Speed, 0..100.
    };
    static_assert(sizeof(AutobuildSettings) == 12, "sizeof AutobuildSettings");

    struct PropertyHeader {
        UInt16_t id;
        UInt16_t numProperties;
    };
    static_assert(sizeof(PropertyHeader) == 4, "sizeof PropertyHeader");

    struct UnitScoreHeader {
        String50_t name;
        UInt16_t scoreType;
        UInt16_t scoreLimit;
    };
    static_assert(sizeof(UnitScoreHeader) == 54, "sizeof UnitScoreHeader");

    struct UnitScoreEntry {
        UInt16_t id;
        UInt16_t score;
        UInt16_t turn;
    };
    static_assert(sizeof(UnitScoreEntry) == 6, "sizeof UnitScoreEntry");

} } }

#endif
