/**
  *  \file game/map/universe.hpp
  */
#ifndef C2NG_GAME_MAP_UNIVERSE_HPP
#define C2NG_GAME_MAP_UNIVERSE_HPP

#include <memory>
#include "game/map/objectvector.hpp"
#include "afl/base/signal.hpp"
#include "game/map/configuration.hpp"
#include "game/interpreterinterface.hpp"
#include "game/map/object.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/playedshiptype.hpp"
#include "game/map/playedplanettype.hpp"
#include "game/map/playedbasetype.hpp"
#include "game/map/ionstormtype.hpp"
#include "game/map/minefieldtype.hpp"
#include "game/map/ufotype.hpp"
#include "game/map/drawingcontainer.hpp"
#include "game/map/fleettype.hpp"
#include "game/map/explosiontype.hpp"
#include "game/reference.hpp"

namespace game { namespace map {

    class Planet;
    class Ship;
    class IonStorm;
    class Reverter;

    class Universe {
     public:
        static const int NameOrbit   = 1;  // locs_Orbit   = 1,           ///< Show "Orbit of" for planet names.
        static const int NameVerbose = 2;  // locs_Verbose = 2,           ///< Be more verbose.
        static const int NameShips   = 4;  // locs_Ships   = 4,           ///< Show a ship name if applicable.
        static const int NameGravity = 8;  // locs_WW      = 8,           ///< Show planet name if point is in warp well.
        static const int NameNoSpace = 16; // locs_NoSpace = 16           ///< Show nothing at all when in deep space.

        Universe();
        ~Universe();

        ObjectVector<Ship>& ships();
        const ObjectVector<Ship>& ships() const;

        PlayedShipType& playedShips();
        const PlayedShipType& playedShips() const;

        ObjectVector<Planet>& planets();
        const ObjectVector<Planet>& planets() const;

        PlayedPlanetType& playedPlanets();
        PlayedBaseType& playedBases();
        FleetType& fleets();

        ObjectVector<IonStorm>& ionStorms();
        const ObjectVector<IonStorm>& ionStorms() const;
        IonStormType& ionStormType();

        MinefieldType& minefields();
        const MinefieldType& minefields() const;

        UfoType& ufos();
        const UfoType& ufos() const;

        ExplosionType& explosions();
        const ExplosionType& explosions() const;

        DrawingContainer& drawings();
        const DrawingContainer& drawings() const;

        Configuration& config();
        const Configuration& config() const;

        void setNewReverter(Reverter* p);
        Reverter* getReverter() const;

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
            - signal sig_setChange on all containes so cursors can adapt

            \param playingSet       Set of players we're playing.
                                    Those players will be set to the given \c playability;
                                    others will at best be ReadOnly.
            \param availablePlayers Available players (set of loaded result files).
                                    Used for hasFullData().
            \param playability      Playability to use for players in \c playingSet.
            \param host             Host version
            \param config           Host configuration
            \param turnNumber       Current turn number
            \param shipList         Ship list
            \param tx               Translator (for logging)
            \param log              Logger */
        void postprocess(PlayerSet_t playingSet, PlayerSet_t availablePlayers, Object::Playability playability,
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
            \param gravityFlag  true to look into warp well. If false, function behaves exactly like findPlanetAt(int).
            \param config       Host configuration
            \param host         Host version
            \return Id of planet, or zero if none */
        Id_t findPlanetAt(Point pt,
                          bool gravityFlag,
                          const game::config::HostConfiguration& config,
                          const HostVersion& host) const;

        /** Find planet from warp well location.
            \param pt Location
            \param config       Host configuration
            \param host         Host version
            \pre findPlanetAt(pt) == 0
            \return Id of planet if pt is in its warp wells, 0 otherwise */
        Id_t findGravityPlanetAt(Point pt,
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
            \param config  Host configuration
            \param host    Host version
            \param tx      Translator
            \return name */
        String_t findLocationName(Point pt, int flags,
                                  const game::config::HostConfiguration& config,
                                  const HostVersion& host,
                                  afl::string::Translator& tx) const;

        /** Get names of units at a point in human-readable form.
            \param pt               Location
            \param viewpointPlayer  Viewpoint player (determines whose ships are shown by name)
            \param players          Player list
            \param tx               Translator
            \param iface            Interface (for name retrieval)
            \return human-readable, multi-line string */
        String_t findLocationUnitNames(Point pt,
                                       int viewpointPlayer,
                                       const PlayerList& players,
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
            \return controlling planet Id; 0 if none */
        Id_t findControllingPlanetId(const Minefield& mf) const;

        /** Mark objects within a range of coordinates.
            Coordinates describe a rectangle and can be in any order
            \param a First coordinates
            \param b Second (opposite) coordinates
            \return Number of objects found */
        int markObjectsInRange(Point a, Point b);

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
        Configuration m_config;
        ObjectVector<Ship> m_ships;
        ObjectVector<Planet> m_planets;
        ObjectVector<IonStorm> m_ionStorms;
        std::auto_ptr<MinefieldType> m_minefields;
        std::auto_ptr<UfoType> m_ufos;
        std::auto_ptr<ExplosionType> m_explosions;
        DrawingContainer m_drawings;

        bool m_universeChanged;

        std::auto_ptr<PlayedShipType> m_playedShips;
        std::auto_ptr<PlayedPlanetType> m_playedPlanets;
        std::auto_ptr<PlayedBaseType> m_playedBases;
        std::auto_ptr<FleetType> m_fleets;
        std::auto_ptr<IonStormType> m_ionStormType;

        std::auto_ptr<Reverter> m_reverter;

        PlayerSet_t m_availablePlayers;     // ex data_set
    };

} }

#endif
