/**
  *  \file gfx/threed/positionlist.hpp
  *  \brief Class gfx::threed::PositionList
  */
#ifndef C2NG_GFX_THREED_POSITIONLIST_HPP
#define C2NG_GFX_THREED_POSITIONLIST_HPP

#include <vector>
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "gfx/threed/vecmath.hpp"

namespace gfx { namespace threed {

    /** List of positions.
        A 3-D model can come with associated positions, to mark specific points in a model
        (e.g. mountpoint for a beam, used as origin of a beam being fired).

        This data class contains a list of Id/position associations.
        The Id defines the type of the point; Ids can appear multiple times, order is significant.

        Id=0 should not be used in a point definition. */
    class PositionList {
     public:
        /** Definition for a position set.
            @see findPoints() */
        struct Definition {
            uint16_t itemId;         ///< Id for a single position.
            uint16_t firstId;        ///< Id for start of a range.
            uint16_t lastId;         ///< Id for end of a range. Set to same as firstId if there is no range definition.
        };

        /** Constructor.
            Makes an empty list. */
        PositionList();

        /** Destructor. */
        ~PositionList();

        /** Add an item.
            @param id   Id; should not be 0.
            @param pos  Position */
        void add(uint16_t id, const Vec3f& pos);

        /** Get number of positions.
            @return number of positions */
        size_t getNumPositions() const;

        /** Get Id, given an index.
            @param index Index [0,getNumPositions())
            @return Id; 0 if index is out of range */
        uint16_t getIdByIndex(size_t index) const;

        /** Get position, given an index.
            @param index Index [0,getNumPositions())
            @return position; null vector if index is out of range */
        Vec3f getPositionByIndex(size_t index) const;

        /** Find Id, given an index.
            @param id      Id
            @param startAt First position to look at (for repeated find)
            @return 0-based index, if found */
        afl::base::Optional<size_t> findId(uint16_t id, size_t startAt = 0) const;

        /** Find points of a position set.
            The point Ids are defined using a Definition structure,
            defining either individual positions, or position ranges.
            If not enough individual positions are provided, the position ranges are interpolated to produce as more points as required.
            If too many points are generated, discards the excess.

            @param def Definition
            @param numPoints Number of points requested
            @return Found points; array of [0, numPoints] elements */
        std::vector<Vec3f> findPoints(const Definition def, size_t numPoints) const;

     private:
        struct Item {
            uint16_t id;
            Vec3f pos;

            Item(uint16_t id, Vec3f pos)
                : id(id), pos(pos)
                { }
        };
        std::vector<Item> m_items;
    };

} }

#endif
