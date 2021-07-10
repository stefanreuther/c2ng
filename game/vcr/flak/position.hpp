/**
  *  \file game/vcr/flak/position.hpp
  *  \brief Class game::vcr::flak::Position
  */
#ifndef C2NG_GAME_VCR_FLAK_POSITION_HPP
#define C2NG_GAME_VCR_FLAK_POSITION_HPP

#include "afl/base/types.hpp"

namespace game { namespace vcr { namespace flak {

    /** Position in 3-D space.
        Note that although the engine produces 3-D coordinates, rules are 2-D only.
        (In the same way a classic VCR produces 2-D, but actually is only 1-D.) */
    struct Position {
        // ex FlakPos
        int32_t x, y, z;

        Position()
            : x(0), y(0), z(0)
            { }

        Position(int32_t x, int32_t y, int32_t z)
            : x(x), y(y), z(z) { }

        /** Compute distance between two positions.
            Ignores the Z coordinates.
            \param other Other position
            \return distance */
        double distanceTo(const Position& other) const;

        /** Check whether distance between two positions is smaller than distance.
            \param other Other position.
            \param radius Distance to check
            \return true if distance is less or equal */
        bool isDistanceLERadius(const Position& other, int32_t radius) const;

        /** Compare for equality.
            \param pos Other position */
        bool operator==(const Position& pos) const;

        /** Compare for inequality.
            \param pos Other position */
        bool operator!=(const Position& pos) const
            { return !operator==(pos); }
    };

} } }

#endif
