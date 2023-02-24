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
        /** Constructor.
            \param id Object Id. The object Id is the value returned by getId(), and usually remains unchanged
                      during the object's lifetime. If needed, it can be changed using setId(). */
        explicit CircularObject(Id_t id)
            : Object(id)
            { }

        /** Virtual destructor. */
        virtual ~CircularObject();

        /** Get object radius in light-years.
            @return radius in ly */
        virtual afl::base::Optional<int> getRadius() const = 0;

        /** Get squared radius in light-years.
            This is used for objects that have fractional radius, such as minefields.
            @return squared radius in ly^2 */
        virtual afl::base::Optional<int32_t> getRadiusSquared() const = 0;
    };


} }

#endif
