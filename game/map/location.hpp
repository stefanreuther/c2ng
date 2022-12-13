/**
  *  \file game/map/location.hpp
  *  \brief Class game::map::Location
  */
#ifndef C2NG_GAME_MAP_LOCATION_HPP
#define C2NG_GAME_MAP_LOCATION_HPP

#include "afl/base/signal.hpp"
#include "afl/bits/smallset.hpp"
#include "game/map/point.hpp"
#include "game/reference.hpp"

namespace game { namespace map {

    class Universe;
    class Configuration;

    /** Symbolic map location.

        Represents a location given either as a coordinate, or an object reference.
        An object reference tracks the object even if it changes position.
        If it disappears (because it is not visible in a turn), we remain at the last position.

        Basic support exists for tracking objects across a wrap border:
        when you do set(Point) for an alias of an object's position, then set(Reference) for an object reference,
        the position will be reported as that point alias. */
    class Location {
     public:
        /** Flag for browse(). */
        enum BrowseFlag {
            Backwards,           ///< Browse backwards (towards lower Ids) instead of forward.
            MarkedOnly,          ///< Accept only marked objects.
            PlayedOnly           ///< Accept only played objects (playability ReadOnly or better) if starting from played object.
        };
        typedef afl::bits::SmallSet<BrowseFlag> BrowseFlags_t;

        /** Default constructor.
            Creates a location that has no position. */
        Location();

        /** Set universe.
            \param univ Universe. Must live longer than the Location object. Can be null.
            \param mapConfig Map configuration. Must live longer than the Location object. Can be null. */
        void setUniverse(Universe* univ, const Configuration* mapConfig);

        /** Set location to reference.
            If the location refers to a map object, we start tracking this object.
            \param ref Reference */
        void set(Reference ref);

        /** Set location to fixed position.
            \param pt Point */
        void set(Point pt);

        /** Get position.
            \return position if known */
        afl::base::Optional<Point> getPosition() const;

        /** Get possible "other end" position.
            This is a suggested position for the user to go to.
            - if position is at ship, return its waypoint
            - if position is at ship waypoint, return its position
            - if position is in a wormhole, return exit position
            - if circular map is active, switch between map images
            \param shipId Focus ship Id; can be 0
            \return alternate position */
        afl::base::Optional<Point> getOtherPosition(Id_t shipId) const;

        /** Get reference.
            \return reference */
        Reference getReference() const;

        /** Get effective reference.
            If the reference set using set(Reference) controls the position of this Location,
            returns that; otherwise, returns an unset reference.
            \return reference */
        Reference getEffectiveReference() const;

        /** Browse.
            If this location is controlled by a Reference, browses to the next object of its type.
            \param flags Flags to choose direction and object subset to iterate through */
        void browse(BrowseFlags_t flags);

        /** Signal: position change.
            Raised whenever set() sets a new position.
            As of 20181231, this does NOT signal implicit changes through change of the reference/universe. */
        afl::base::Signal<void(Point)> sig_positionChange;

     private:
        Universe* m_pUniverse;
        const Configuration* m_pConfig;
        Point m_point;
        Reference m_reference;
        bool m_pointValid;

        void notifyListeners(const afl::base::Optional<Point>& lastPos);
    };

} }

#endif
