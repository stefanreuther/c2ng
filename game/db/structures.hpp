/**
  *  \file game/db/structures.hpp
  *  \brief Database File Structures
  */
#ifndef C2NG_GAME_DB_STRUCTURES_HPP
#define C2NG_GAME_DB_STRUCTURES_HPP

#include "afl/base/staticassert.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "game/types.hpp"
#include "game/v3/structures.hpp"

namespace game { namespace db { namespace structures {

    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;
    typedef afl::bits::Value<afl::bits::Int32LE> Int32_t;
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

    /// Planet history record (rPlanetHistory, 1).
    struct Planet {
        game::v3::structures::Planet planet;                    ///< Planet data.
        Int16_t     turn[4];                                    ///< Timestamps. @see DatabasePlanetTimestamp.
        uint8_t     knownToHaveNatives;                         ///< true if we know this planet has natives.
    };
    static_assert(sizeof(Planet) == 94, "sizeof Planet");

    /// Indexes for Planet::turn.
    enum PlanetTimestamp {
        PlanetMinerals,                                         ///< Mined/ground/density fields.
        PlanetColonists,                                        ///< Population/owner/industry fields.
        PlanetNatives,                                          ///< Native gov/pop/race fields.
        PlanetCash                                              ///< Cash/supplies fields.
    };

    /// Ship history record (rShipHistory, 2).
    struct Ship {
        game::v3::structures::Ship ship;                        ///< Ship data.
        Int16_t     turn[2];                                    ///< Timestamps. @see ShipTimestamp.
    };
    static_assert(sizeof(Ship) == 111, "sizeof Ship");

    /** Indexes for Ship::turn. */
    enum ShipTimestamp {
        ShipArmsDamage,                                         ///< Arms/damage.
        ShipRest                                                ///< Cargo etc.
    };

    /// Ship Track entry (part of rShipTrack, 3).
    struct ShipTrackEntry {
        Int16_t     x, y;                                       ///< Ship position.
        int8_t      speed;                                      ///< Ship speed.
        Int16_t     heading;                                    ///< Ship heading (angle, degrees).
        Int16_t     mass;                                       ///< Ship mass.
    };
    static_assert(sizeof(ShipTrackEntry) == 9, "sizeof ShipTrackEntry");

    /// Ship Track header (rShipTrack, 3). Followed by multiple ShipTrackEntry.
    struct ShipTrackHeader {
        Int16_t     id;                                         ///< Ship Id.
        Int16_t     turn;                                       ///< Reference turn, i.e.\ turn of first ShipTrackEntry that follows (entries in reverse chronological order).
    };
    static_assert(sizeof(ShipTrackHeader) == 4, "sizeof ShipTrackHeader");

    /// Minefield History Record (rMinefield, 4).
    struct Minefield {
        Int16_t     id;                                         ///< Minefield Id.
        Int16_t     x, y;                                       ///< Minefield center.
        Int16_t     owner;                                      ///< Minefield owner.
        Int32_t     units;                                      ///< Minefield units.
        Int16_t     type;                                       ///< Minefield type: 0=normal, 1=web.
        Int16_t     turn;                                       ///< Turn number for which this information holds.
    };
    static_assert(sizeof(Minefield) == 16, "sizeof Minefield");

    /// User drawing (rPainting, 5).
    struct Drawing {
        uint8_t     type;                                       ///< Painting type, and comment flag.
        uint8_t     color;                                      ///< Painting color.
        Int16_t     x1, y1;                                     ///< Left-top or center position.
        Int16_t     x2, y2;                                     ///< Bottom-right position, radius or shape.
        Int16_t     tag;                                        ///< User-defined tag.
        Int16_t     expirationTurn;                             ///< Turn of expiry.
    };
    static_assert(sizeof(Drawing) == 14, "sizeof Drawing");

    /// Autobuild settings (rAutoBuild, 6).
    struct AutobuildSettings {
        Int16_t     goal[4];                                    ///< Target number, 0..999; 1000 meaning "max".
        int8_t      speed[4];                                   ///< Speed, 0..100.
    };
    static_assert(sizeof(AutobuildSettings) == 12, "sizeof AutobuildSettings");
    static_assert(NUM_PLANETARY_BUILDING_TYPES == 4, "NUM_PLANETARY_BUILDING_TYPES");

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

    /** Ufo history (rUfoHistory, 12). */
    struct Ufo {
        Int16_t     id;                                         ///< Ufo Id.
        game::v3::structures::Ufo ufo;                          ///< Ufo data as last seen.
        Int32_t     realId;                                     ///< Real ID of object represented by Ufo.
        Int16_t     turnLastSeen;                               ///< Turn in which Ufo was last seen.
        Int16_t     xLastSeen, yLastSeen;                       ///< Location at which Ufo was last seen.
        Int16_t     speedX, speedY;                             ///< Movement vector, if known.
    };
    static_assert(sizeof(Ufo) == 94, "sizeof Ufo");

} } }

#endif
