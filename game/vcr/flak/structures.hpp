/**
  *  \file game/vcr/flak/structures.hpp
  *  \brief FLAK Structures
  */
#ifndef C2NG_GAME_VCR_FLAK_STRUCTURES_HPP
#define C2NG_GAME_VCR_FLAK_STRUCTURES_HPP

#include "afl/base/staticassert.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/value.hpp"

namespace game { namespace vcr { namespace flak { namespace structures {

    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;
    typedef afl::bits::Value<afl::bits::Int32LE> Int32_t;
    typedef afl::bits::Value<afl::bits::FixedString<20> > String20_t;

    /** Magic number for FLAK VCR files. */
    const char FLAK_MAGIC[8] = { 'F','L','A','K','V','C','R',26 };

    /** FLAK flags.
        Values for Ship::flags. */
    const int flak_IsPlanet = 1;               ///< Set if this is a planet, not a ship.

    /** FLAK Ship Data. */
    struct Ship {
        // ex TFlakShip
        String20_t name;                       ///< Name.
        Int16_t    damage;                     ///< Initial damage. ex damage_init.
        Int16_t    crew;                       ///< Initial crew. ex crew_init.
        Int16_t    id;                         ///< Id number.
        Int16_t    owner;                      ///< Player number. ex player.
        Int16_t    hull;                       ///< Hull type.
        Int16_t    experienceLevel;            ///< Experience level. ex level.
        Int16_t    numBeams;                   ///< Number of beams. ex beam_count.
        Int16_t    beamType;                   ///< Beam type. ex beam_type.
        Int16_t    numLaunchers;               ///< Number of torpedo launchers. ex torp_lcount.
        Int16_t    numTorpedoes;               ///< Number of torpedoes. ex torp_count.
        Int16_t    torpedoType;                ///< Torpedo type. ex torp_type.
        Int16_t    numBays;                    ///< Number of fighter bays. ex bay_count.
        Int16_t    numFighters;                ///< Number of fighters. ex fighter_count.
        Int16_t    mass;                       ///< Combat mass.
        Int16_t    shield;                     ///< Initial shields. ex shield_init.
        Int16_t    maxFightersLaunched;        ///< Maximum number of simultaneously launched fighters allowed. ex max_fighters_launched.
        Int32_t    rating;                     ///< Targeting rating.
        Int16_t    compensation;               ///< Compensation rating. ex compensation_rating.
        Int16_t    flags;                      ///< Additional flags.
        Int16_t    endingStatus;               ///< Ending status.
    };
    static_assert(sizeof(Ship) == 62, "sizeof Ship");

    /** FLAK Fleet Data. */
    struct Fleet {
        // ex TFlakFleet
        Int16_t    owner;                      ///< Player number. ex player.
        Int16_t    firstShipIndex;             ///< Index of first ship (0-based). ex first_ship.
        Int16_t    numShips;                   ///< Number of ships. ex num_ships.
        Int16_t    speed;                      ///< Speed.
        Int32_t    firstAttackListIndex;       ///< Index of first attack list entry (0-based). ex att_list_pointer.
        Int32_t    numAttackListEntries;       ///< Number of attack list entries. ex att_list_size.
        Int32_t    x, y;                       ///< Initial position (combat coordinates, meters). ex x_init, y_init.
    };
    static_assert(sizeof(Fleet) == 24, "sizeof Fleet");

    /** FLAK Battle Data. */
    struct Battle {
        // ex TFlakBattle
        Int32_t    this_size;                  ///< Total size in file.
        Int16_t    x, y;                       ///< Location in universe (starchart coordinates, ly).
        Int32_t    seed;                       ///< Random number seed.
        Int32_t    total_time;                 ///< Total time required to resolve this fight.
        Int32_t    ambient_flags;              ///< Ambient flags.

        Int32_t    num_fleets;                 ///< Number of fleets.
        Int32_t    fleet_entry_size;           ///< Size of a fleet entry. Equals Fleet::size.
        Int32_t    fleet_ptr;                  ///< Position of first fleet, relative to start of this record.

        Int32_t    num_ships;                  ///< Number of ships.
        Int32_t    ship_entry_size;            ///< Size of a ship entry. Equals Ship::size.
        Int32_t    ship_ptr;                   ///< Position of first ship, relative to start of this record.

        Int32_t    num_att_list_entries;       ///< Number of attack list entries.
        Int32_t    att_list_entry_size;        ///< Size of an attack list entry. Set to 4 (=2 int16's).
        Int32_t    att_list_ptr;               ///< Position of first attack list entry, relative to start of this record.
    };
    static_assert(sizeof(Battle) == 56, "sizeof Battle");

    /** FLAK File Header.
        A FLAK file starts with this record, and is followed by a handful of Battle records. */
    struct Header {
        // ex TFlakHeader
        char       magic[8];                   ///< Magic number.
        Int16_t    filefmt_version;            ///< File format version.
        Int16_t    player;                     ///< Player number. Addressee of this file.
        Int16_t    turn;                       ///< Turn number.
        Int16_t    num_battles;                ///< Number of battles.
        uint8_t    timestamp[18];              ///< Timestamp.
        Int32_t    reserved;                   ///< Reserved for future expansion.
    };
    static_assert(sizeof(Header) == 38, "sizeof Header");

} } } }

#endif
