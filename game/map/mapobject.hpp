/**
  *  \file game/map/mapobject.hpp
  */
#ifndef C2NG_GAME_MAP_MAPOBJECT_HPP
#define C2NG_GAME_MAP_MAPOBJECT_HPP

#include "game/map/point.hpp"
#include "game/map/object.hpp"

namespace game { namespace map {
    
    /** Game object on the map.
        This extends Object to include a map position. */
    class MapObject : public Object {
     public:
        /** Destructor. */
        virtual ~MapObject();

        /** Get position in game universe. */
        virtual bool getPosition(Point& result) const = 0;
    };

} }

#endif
