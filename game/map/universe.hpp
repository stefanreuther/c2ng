/**
  *  \file game/map/universe.hpp
  *  \brief Class game::map::Universe
  */
#ifndef C2NG_GAME_MAP_UNIVERSE_HPP
#define C2NG_GAME_MAP_UNIVERSE_HPP

#include <memory>
#include "afl/base/signal.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/interpreterinterface.hpp"
#include "game/map/drawingcontainer.hpp"
#include "game/map/explosiontype.hpp"
#include "game/map/fleettype.hpp"
#include "game/map/ionstormtype.hpp"
#include "game/map/minefieldtype.hpp"
#include "game/map/object.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/playedbasetype.hpp"
#include "game/map/playedplanettype.hpp"
#include "game/map/playedshiptype.hpp"
#include "game/map/ufotype.hpp"
#include "game/reference.hpp"

namespace game { namespace map {

    class Configuration;
    class Planet;
    class Ship;
    class IonStorm;
    class Reverter;

    /** Universe.
        Serves as container for all sorts of map objects; owns those objects.
        It contains:
        - object containers (ObjectVector or special classes);
        - ObjectType descendants for everything that has an ObjectCursor;
        - a set of players that have reliable data (hasFullData),
          used to implement "if I don't see it, it's gone" logic;
        - an optional Reverter to undo one-way operations;
        - listener logic. */
    class Universe {
     public:
        /*
         *  Flags for findLocationName()
         */
        static const int NameOrbit   = 1;  // locs_Orbit   = 1,           ///< Show "Orbit of" for planet names.
        static const int NameVerbose = 2;  // locs_Verbose = 2,           ///< Be more verbose.
        static const int NameShips   = 4;  // locs_Ships   = 4,           ///< Show a ship name if applicable.
        static const int NameGravity = 8;  // locs_WW      = 8,           ///< Show planet name if point is in warp well.
        static const int NameNoSpace = 16; // locs_NoSpace = 16           ///< Show nothing at all when in deep space.

        /** Constructor.
            Make an empty universe. */
        Universe();

        /** Destructor. */
        ~Universe();

        /** Access ships.
            \return ship vector */
        ObjectVector<Ship>& ships();
        const ObjectVector<Ship>& ships() const;

        /** Access played ships.
            \return PlayedShipType */
        PlayedShipType& playedShips();
        const PlayedShipType& playedShips() const;

        /** Access planets.
            \return planet vector */
        ObjectVector<Planet>& planets();
        const ObjectVector<Planet>& planets() const;

        /** Access played planets.
            \return PlayedPlanetType */
        PlayedPlanetType& playedPlanets();
        const PlayedPlanetType& playedPlanets() const;

        /** Access played bases.
            \return PlayedBaseType */
        PlayedBaseType& playedBases();
        const PlayedBaseType& playedBases() const;

        /** Access fleets.
            \return fleets */
        FleetType& fleets();
        const FleetType& fleets() const;

        /** Access ion storms.
            \return ion storm vector */
        ObjectVector<IonStorm>& ionStorms();
        const ObjectVector<IonStorm>& ionStorms() const;

        /** Access ion storms.
            \return IonStormType */
        IonStormType& ionStormType();
        const IonStormType& ionStormType() const;

        /** Access minefields.
            \return MinefieldType */
        MinefieldType& minefields();
        const MinefieldType& minefields() const;

        /** Access Ufos.
            \return UfoType */
        UfoType& ufos();
        const UfoType& ufos() const;

        /** Access explosions.
            \return ExplosionType */
        ExplosionType& explosions();
        const ExplosionType& explosions() const;

        /** Access drawings.
            \return DrawingContainer */
        DrawingContainer& drawings();
        const DrawingContainer& drawings() const;

        /** Set reverter.
            The Universe will own the Reverter instance.
            Setting a Reverter will replace the previous one.
            \param p Newly-allocated reverter; can be null */
        void setNewReverter(Reverter* p);

        /** Access reverter.
            \return Reverter; can be null */
        Reverter* getReverter() const;

        /** Resolve Reference into an Object.
            \param ref Reference
            \return Object, if reference refers to a valid object; otherwise, null */
        const Object* getObject(Reference ref) const;
        Object* getObject(Reference ref);

        /** Perform all updates.
            This will poll all updatable objects, and raise the appropriate signals:
            - sig_preUpdate
            - all objects' sig_change
            - sig_universeChange (if needed) */
        void notifyListeners();

        /** Mark universe changed. */
        void markChanged();

        /** Postprocess universe.
            Call this to make structural changes propagate.
            In particular, this calls
            - objects' internalCheck methods
            - objects' combinedCheck methods
            - set objects' playability
            - signal sig_setChange on all containers so cursors can adapt

            \param playingSet       Set of players we're playing.
                                    Those players will be set to the given \c playability;
                                    others will at best be ReadOnly.
            \param availablePlayers Available players (set of loaded result files).
                                    Used for hasFullData().
            \param playability      Playability to use for players in \c playingSet.
            \param mapConfig        Map configuration
            \param host             Host version
            \param config           Host configuration
            \param turnNumber       Current turn number
            \param shipList         Ship list
            \param tx               Translator (for logging)
            \param log              Logger */
        void postprocess(PlayerSet_t playingSet, PlayerSet_t availablePlayers, Object::Playability playability,
                         const Configuration& mapConfig,
                         const game::HostVersion& host, const game::config::HostConfiguration& config,
                         int turnNumber,
                         const game::spec::ShipList& shipList,
                         afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Check for full data.
            \param playerNr Player number to check
            \return true if we have a result file for this player loaded */
        bool hasFullData(int playerNr) const;


        /*
         *  Location accessors
         */

        /** Find planet at location.
            \param pt Location, need not be normalized
            \return Id of planet, or zero if none */
        Id_t findPlanetAt(Point pt) const;

        /** Find planet at location, with optional warp wells.
            If there is no planet at the location, look whether the point is in the warp well of one.
            \param pt           Location, need not be normalized
            \param gravityFlag  true to look into warp well. If false, function behaves exactly like findPlanetAt(Point).
            \param mapConfig    Map configuration
            \param config       Host configuration
            \param host         Host version
            \return Id of planet, or zero if none */
        Id_t findPlanetAt(Point pt,
                          bool gravityFlag,
                          const game::map::Configuration& mapConfig,
                          const game::config::HostConfiguration& config,
                          const HostVersion& host) const;

        /** Find planet from warp well location.
            \param pt Location
            \param mapConfig    Map configuration
            \param config       Host configuration
            \param host         Host version
            \pre findPlanetAt(pt) == 0
            \return Id of planet if pt is in its warp wells, 0 otherwise */
        Id_t findGravityPlanetAt(Point pt,
                                 const game::map::Configuration& mapConfig,
                                 const game::config::HostConfiguration& config,
                                 const HostVersion& host) const;

        /** Get ship at position. Any race does.
            This is mainly used for naming locations, and for things like to tell whether a "L" command would succeed.
            \param pt position to check
            \returns Id number of a ship at position \c pt, or zero if none. */
        Id_t findFirstShipAt(Point pt) const;

        /** Get name of a location in human-readable form.
            \param pt      location
            \param flags   details of requested string (NameShips, NameGravity, NameVerbose, NameNoSpace)
            \param mapConfig Map configuration
            \param config  Host configuration
            \param host    Host version
            \param tx      Translator
            \return name */
        String_t findLocationName(Point pt, int flags,
                                  const game::map::Configuration& mapConfig,
                                  const game::config::HostConfiguration& config,
                                  const HostVersion& host,
                                  afl::string::Translator& tx) const;

        /** Get names of units at a point in human-readable form.
            \param pt               Location
            \param viewpointPlayer  Viewpoint player (determines whose ships are shown by name)
            \param players          Player list
            \param mapConfig        Map configuration
            \param tx               Translator
            \param iface            Interface (for name retrieval)
            \return human-readable, multi-line string */
        String_t findLocationUnitNames(Point pt,
                                       int viewpointPlayer,
                                       const PlayerList& players,
                                       const game::map::Configuration& mapConfig,
                                       afl::string::Translator& tx,
                                       InterpreterInterface& iface) const;

        /** Check whether a ship is being towed.
            \param sid Ship Id
            \param after Start searching after this Id
            \return Id, 0 if none */
        Id_t findShipTowing(Id_t sid, Id_t after = 0) const;

        /** Find ship cloning at a given planet.
            \param pid Planet Id
            \param after Start searching after this ship Id
            \return Id, 0 if none */
        Id_t findShipCloningAt(Id_t pid, Id_t after = 0) const;

        /** Find planet controlling a minefield.
            \param mf Minefield
            \param mapConfig Map configuration
            \return controlling planet Id; 0 if none */
        Id_t findControllingPlanetId(const Minefield& mf, const Configuration& mapConfig) const;

        /** Find planet with universal minefield friendly code (mfX).
            \param forPlayer Player; must have played planets
            \return planet Id; 0 if none */
        Id_t findUniversalMinefieldFriendlyCodePlanetId(int forPlayer) const;

        /** Mark objects within a range of coordinates.
            Coordinates describe a rectangle and can be in any order
            \param a First coordinates
            \param b Second (opposite) coordinates
            \param mapConfig Map configuration
            \return Number of objects found */
        int markObjectsInRange(Point a, Point b, const game::map::Configuration& mapConfig);

        /** Signal: about to update.
            Raised before checking to raise any object's sig_change (even if no signal is eventually raised).

            Essentially, this says: if you get a bunch of Object::sig_change events,
            you can wait for a sig_universeChange to commit the transaction. */
        afl::base::Signal<void()> sig_preUpdate;

        /** Signal: universe changed.
            Raised after any object's sig_change has been raised, or the universe itself was dirty (markChanged()).
            Unlike sig_preUpdate, this is only raised when there were any changes. */
        afl::base::Signal<void()> sig_universeChange;

     private:
        // Object containers
        ObjectVector<Ship> m_ships;
        ObjectVector<Planet> m_planets;
        ObjectVector<IonStorm> m_ionStorms;
        MinefieldType m_minefields;
        UfoType m_ufos;
        ExplosionType m_explosions;
        DrawingContainer m_drawings;

        // Change tracking
        bool m_universeChanged;

        // Types (required for everything that has a cursor)
        PlayedShipType m_playedShips;
        PlayedPlanetType m_playedPlanets;
        PlayedBaseType m_playedBases;
        FleetType m_fleets;
        IonStormType m_ionStormType;

        // Reverter
        std::auto_ptr<Reverter> m_reverter;

        // Set of players that have reliable data
        PlayerSet_t m_availablePlayers;     // ex data_set
    };

} }

inline game::map::ObjectVector<game::map::Ship>&
game::map::Universe::ships()
{
    // ex GUniverse::getShip, GUniverse::isValidShipId (sort-of)
    return m_ships;
}

inline const game::map::ObjectVector<game::map::Ship>&
game::map::Universe::ships() const
{
    return m_ships;
}

inline game::map::PlayedShipType&
game::map::Universe::playedShips()
{
    return m_playedShips;
}

inline const game::map::PlayedShipType&
game::map::Universe::playedShips() const
{
    return m_playedShips;
}

inline game::map::ObjectVector<game::map::Planet>&
game::map::Universe::planets()
{
    // ex GUniverse::getPlanet, GUniverse::isValidPlanetId (sort-of)
    return m_planets;
}

inline const game::map::ObjectVector<game::map::Planet>&
game::map::Universe::planets() const
{
    return m_planets;
}

inline game::map::PlayedPlanetType&
game::map::Universe::playedPlanets()
{
    return m_playedPlanets;
}

inline const game::map::PlayedPlanetType&
game::map::Universe::playedPlanets() const
{
    return m_playedPlanets;
}

inline game::map::PlayedBaseType&
game::map::Universe::playedBases()
{
    return m_playedBases;
}

const inline game::map::PlayedBaseType&
game::map::Universe::playedBases() const
{
    return m_playedBases;
}

inline game::map::FleetType&
game::map::Universe::fleets()
{
    return m_fleets;
}

inline const game::map::FleetType&
game::map::Universe::fleets() const
{
    return m_fleets;
}

inline game::map::ObjectVector<game::map::IonStorm>&
game::map::Universe::ionStorms()
{
    // ex GUniverse::isValidIonStormId, GUniverse::getIonStorm (sort-of)
    return m_ionStorms;
}

inline const game::map::ObjectVector<game::map::IonStorm>&
game::map::Universe::ionStorms() const
{
    return m_ionStorms;
}

inline game::map::IonStormType&
game::map::Universe::ionStormType()
{
    return m_ionStormType;
}

inline const game::map::IonStormType&
game::map::Universe::ionStormType() const
{
    return m_ionStormType;
}

inline game::map::MinefieldType&
game::map::Universe::minefields()
{
    return m_minefields;
}

inline const game::map::MinefieldType&
game::map::Universe::minefields() const
{
    return m_minefields;
}

inline game::map::UfoType&
game::map::Universe::ufos()
{
    return m_ufos;
}

inline const game::map::UfoType&
game::map::Universe::ufos() const
{
    return m_ufos;
}

inline game::map::ExplosionType&
game::map::Universe::explosions()
{
    return m_explosions;
}

inline const game::map::ExplosionType&
game::map::Universe::explosions() const
{
    return m_explosions;
}

inline game::map::DrawingContainer&
game::map::Universe::drawings()
{
    return m_drawings;
}

inline const game::map::DrawingContainer&
game::map::Universe::drawings() const
{
    return m_drawings;
}

inline game::map::Reverter*
game::map::Universe::getReverter() const
{
    return m_reverter.get();
}

inline void
game::map::Universe::markChanged()
{
    m_universeChanged = true;
}

#endif
