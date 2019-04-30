/**
  *  \file game/map/location.hpp
  *  \brief Class game::map::Location
  */
#ifndef C2NG_GAME_MAP_LOCATION_HPP
#define C2NG_GAME_MAP_LOCATION_HPP

#include "game/reference.hpp"
#include "game/map/point.hpp"
#include "afl/base/signal.hpp"

namespace game { namespace map {

    class Universe;

    /** Symbolic map location.

        Represents a location given either as a coordinate, or an object reference.
        An object reference tracks the object even if it changes position.
        If it disappears (because it is not visible in a turn), we remain at the last position. */
    class Location {
     public:
        /** Default constructor.
            Creates a location that has no position. */
        Location();

        /** Set universe.
            \param univ Universe. Must live longer than the Location object. Can be null. */
        void setUniverse(Universe* univ);

        /** Set location to reference.
            If the location refers to a map object, we start tracking this object.
            \param ref Reference */
        void set(Reference ref);

        /** Set location to fixed position.
            \param pt Point */
        void set(Point pt);

        /** Get position.
            \param pt [out] Point
            \return true if position known and has been set */
        bool getPosition(Point& pt) const;

        /** Get reference.
            \return reference */
        Reference getReference() const;

        /** Signal: position change.
            Raised whenever set() sets a new position.
            As of 20181231, this does NOT signal implicit changes through change of the reference/universe. */
        afl::base::Signal<void(Point)> sig_positionChange;

     private:
        Universe* m_pUniverse;
        Point m_point;
        Reference m_reference;
        bool m_pointValid;
    };

} }

#endif
