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

        /** Perform all updates.
            This will poll all updatable objects, and raise the appropriate signals. */
        void notifyListeners();

        /** Mark universe changed. */
        void markChanged();

        void postprocess(PlayerSet_t playingSet, PlayerSet_t availablePlayers, Object::Playability playability,
                         const game::HostVersion& host, const game::config::HostConfiguration& config,
                         int turnNumber,
                         afl::string::Translator& tx, afl::sys::LogListener& log);


        /*
         *  Location accessors
         */
        Id_t getPlanetAt(Point pt) const;
        Id_t getPlanetAt(Point pt,
                         bool gravityFlag,
                         const game::config::HostConfiguration& config,
                         const HostVersion& host) const;
        Id_t getGravityPlanetAt(Point pt,
                                const game::config::HostConfiguration& config,
                                const HostVersion& host) const;

        Id_t getAnyShipAt(Point pt);

        String_t getLocationName(Point pt, int flags,
                                 const game::config::HostConfiguration& config,
                                 const HostVersion& host,
                                 afl::string::Translator& tx,
                                 InterpreterInterface& iface);

        Id_t findShipTowing(int sid, int after = 0) const;

        afl::base::Signal<void()> sig_preUpdate;
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

// class GUniverse {
//  public:
//     // Initialisation
//     void recomputeOrbitFlags();

//     // Location accessors

//     // Updates

//     void addMessageInformation(const GMessageInformation& msgi);


//     GPlayedShipType    ty_played_ships;
//     GAnyShipType       ty_any_ships;
//     GHistoryShipType   ty_history_ships;
//     GPlayedPlanetType  ty_played_planets;
//     GAnyPlanetType     ty_any_planets;
//     GPlayedBaseType    ty_played_bases;

//     GPlayerSet   playing_set, data_set;



    };

} }

#endif
