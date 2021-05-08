/**
  *  \file game/map/rangeset.hpp
  *  \brief Class game::map::RangeSet
  */
#ifndef C2NG_GAME_MAP_RANGESET_HPP
#define C2NG_GAME_MAP_RANGESET_HPP

#include <map>
#include "game/map/point.hpp"
#include "game/playerset.hpp"

namespace game { namespace map {

    class ObjectType;

    /** Set of view ranges.
        Used to determine the area visibly by a player, as circular ranges around their units.
        Stores a list of points and ranges. */
    class RangeSet {
     public:
        struct PointCompare {
            bool operator()(const Point& a, const Point& b)
                {
                    return a.getY() != b.getY()
                        ? a.getY() < b.getY()
                        : a.getX() < b.getX();
                }
        };
        typedef std::map<Point,int,PointCompare> PointMap_t;
        typedef PointMap_t::const_iterator Iterator_t;

        /** Constructor.
            Make an empty set. */
        RangeSet();

        /** Destructor. */
        ~RangeSet();

        /** Add a single range.
            \param pt Center
            \param r Radius */
        void add(Point pt, int r);

        /** Add objects from a type.
            Objects must have a known owner.
            \param type Type to iterate through
            \param playerLimit Only include these players
            \param markedOnly true to only include marked objects
            \param r Radius */
        void addObjectType(ObjectType& type, PlayerSet_t playerLimit, bool markedOnly, int r);

        /** Clear.
            \post isEmpty() */
        void clear();

        /** Chekc emptiness.
            \return true if RangeSet is empty */
        bool isEmpty() const;

        /** Get minimum point of bounding box.
            \return point */
        Point getMin() const;

        /** Get maximum point of bounding box.
            \return point */
        Point getMax() const;

        /** Get begin iterator.
            \return iterator */
        Iterator_t begin() const;

        /** Get end iterator.
            \return iterator */
        Iterator_t end() const;

     private:
        PointMap_t m_points;
        Point m_min;
        Point m_max;
    };

} }

#endif
