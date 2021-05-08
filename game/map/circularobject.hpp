/**
  *  \file game/map/circularobject.hpp
  *  \brief Base class game::map::CircularObject
  */
#ifndef C2NG_GAME_MAP_CIRCULAROBJECT_HPP
#define C2NG_GAME_MAP_CIRCULAROBJECT_HPP

#include "game/map/object.hpp"

namespace game { namespace map {

    /** Circular game object on the map.
        This extends Object to include a radius. */
    class CircularObject : public Object {
     public:
        virtual ~CircularObject();

        /** Get object radius in light-years. */
        virtual bool getRadius(int& result) const = 0;
        
        /** Get squared radius in light-years.
            This is used for objects that have fractional radius, such as minefields. */
        virtual bool getRadiusSquared(int32_t& result) const = 0;
    };


} }

#endif
