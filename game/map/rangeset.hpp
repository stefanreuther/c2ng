/**
  *  \file game/map/rangeset.hpp
  */
#ifndef C2NG_GAME_MAP_RANGESET_HPP
#define C2NG_GAME_MAP_RANGESET_HPP

#include <map>
#include "game/map/point.hpp"
#include "game/playerset.hpp"

namespace game { namespace map {

    class ObjectType;

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

        RangeSet();
        ~RangeSet();

        void add(Point pt, int r);
        void addObjectType(ObjectType& type, PlayerSet_t playerLimit, bool markedOnly, int r);

        void clear();
        bool isEmpty() const;
        Point getMinimum() const;
        Point getMaximum() const;

        Iterator_t begin() const;
        Iterator_t end() const;

     private:
        PointMap_t m_points;
        Point m_minimum;
        Point m_maximum;
    };

} }

#endif
