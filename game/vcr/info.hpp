/**
  *  \file game/vcr/info.hpp
  *  \brief Structures for Object Information
  */
#ifndef C2NG_GAME_VCR_INFO_HPP
#define C2NG_GAME_VCR_INFO_HPP

#include <vector>
#include "util/skincolor.hpp"
#include "afl/string/string.hpp"

namespace game { namespace vcr {

    /** Number of lines in an Info. */
    static const size_t NUM_LINES_PER_UNIT = 4;

    /** Human-readable information about a unit. */
    struct ObjectInfo {
        String_t text[NUM_LINES_PER_UNIT];                    ///< Textual information.
        util::SkinColor::Color color[NUM_LINES_PER_UNIT];     ///< Color hint.
        ObjectInfo();
    };

    /** Information about a group (fleet). */
    struct GroupInfo {
        size_t firstObject;     ///< Index of first object.
        size_t numObjects;      ///< Number of objects in group.
        int32_t x, y;           ///< Starting location. Absolute values in meters, with (0,0) at the center of the battlefield.
        int owner;              ///< Owner.
        int speed;              ///< Movement speed, in meters per battle tick / second.

        GroupInfo(size_t firstObject, size_t numObjects, int32_t x, int32_t y, int owner, int speed)
            : firstObject(firstObject), numObjects(numObjects), x(x), y(y), owner(owner), speed(speed)
            { }
        GroupInfo()
            : firstObject(), numObjects(), x(), y(), owner(), speed()
            { }
    };

    /** Human-readable information about a battle. */
    struct BattleInfo {
        String_t heading;                                     ///< Heading of the battle ("Battle 1 of 10").
        String_t algorithmName;                               ///< Algorithm name ("PHost").
        String_t resultSummary;                               ///< Result summary ("We won").
        String_t position;                                    ///< Position. Can be empty.
        std::vector<ObjectInfo> units;                        ///< Information for all units.
        std::vector<GroupInfo> groups;                        ///< Information for all groups (fleets).
    };

} }

inline
game::vcr::ObjectInfo::ObjectInfo()
{
    for (size_t i = 0; i < NUM_LINES_PER_UNIT; ++i) {
        color[i] = util::SkinColor::Static;
    }
}

#endif
