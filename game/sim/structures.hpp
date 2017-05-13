/**
  *  \file game/sim/structures.hpp
  *  \brief Simulator Structures
  */
#ifndef C2NG_GAME_SIM_STRUCTURES_HPP
#define C2NG_GAME_SIM_STRUCTURES_HPP

#include "game/v3/structures.hpp"
#include "afl/base/staticassert.hpp"

namespace game { namespace sim { namespace structures {

    using game::v3::structures::VcrObject;
    using game::v3::structures::String3_t;
    using game::v3::structures::Int16_t;
    using game::v3::structures::Int32_t;

    using game::v3::structures::NUM_TORPEDO_TYPES;

    const int MAX_VERSION = 5;
    const uint8_t TERMINATOR = 26;
    const size_t MAGIC_LENGTH = 6;

    /** Version 0.
        Followed by 16-bit count, then objects. */
    const uint8_t MAGIC_V0[] = {'C','C','s','i','m',TERMINATOR};
    static_assert(sizeof(MAGIC_V0) == MAGIC_LENGTH, "sizeof MAGIC_V0");

    /** Version 1 and later.
        Followed by 8-bit version ('0' = v1, '1' = v2 etc.), then TERMINATOR,
        then 16-bit count, then objects. */
    const uint8_t MAGIC_V1[] = {'C','C','b','s','i','m'};
    static_assert(sizeof(MAGIC_V1) == MAGIC_LENGTH, "sizeof MAGIC_V1");

    /** Record sizes. */
    const uint8_t RECORD_SIZES[MAX_VERSION+1] = {51,53,53,57,65,67};

    /** CCBSim Ship Structure. */
    struct SimShipData {
        VcrObject   object;
        Int16_t     engineType;
        Int16_t     hullType;
        Int16_t     shield;
        String3_t   friendlyCode;
        Int16_t     aggressiveness;
        Int16_t     mass;
        Int16_t     flags;
        Int32_t     flakRating;
        Int16_t     flakCompensation;
        Int16_t     interceptId;
        Int16_t     flags2;
    };
    static_assert(sizeof(SimShipData) == 67, "sizeof SimShipData");

    struct SimPlanetData {
        Int16_t     numTorpedoes[NUM_TORPEDO_TYPES];            // vcro.name
        Int32_t     _pad0;                                      // vcro.damage, .crew
        Int16_t     id;                                         // vcro.id
        Int16_t     owner;                                      // vcro.owner
        Int16_t     _pad1;                                      // vcro.picture, .hull_or_zero
        Int16_t     beamTechLevel;                              // vcro.beam_type
        uint8_t     _pad2;                                      // vcro.beam_count
        uint8_t     experienceLevel;                            // vcro.experience_level
        Int16_t     numFighters;                                // vcro.bay_count
        Int16_t     _pad3;                                      // vcro.torp_type
        Int16_t     numTorpedoesOld;                            // vcro._ammo
        Int16_t     torpedoTechLevel;                           // vcro.torp_launcher_count
        Int16_t     numBaseDefensePosts;                        // .engine_type
        Int16_t     numDefensePosts;                            // .hull_type
        Int16_t     shield;                                     // .shield
        String3_t   friendlyCode;                               // .fcode
        Int16_t     aggressiveness;                             // .aggressiveness
        Int16_t     _pad5;                                      // .mass
        Int16_t     flags;
        Int32_t     flakRating;
        Int16_t     flakCompensation;
        Int16_t     _pad6;
        Int16_t     flags2;
    };
    static_assert(sizeof(SimPlanetData) == sizeof(SimShipData), "sizeof SimPlanetData");


} } }

#endif
