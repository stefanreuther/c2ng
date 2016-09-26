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
#include "game/timestamp.hpp"

namespace game { namespace map {

    class Planet;
    class Ship;
    class IonStorm;

    class Universe {
     public:
        static const int NameOrbit   = 1;  // locs_Orbit   = 1,           ///< Show "Orbit of" for planet names.
        static const int NameVerbose = 2;  // locs_Verbose = 2,           ///< Be more verbose.
        static const int NameShips   = 4;  // locs_Ships   = 4,           ///< Show a ship name if applicable.
        static const int NameGravity = 8;  // locs_WW      = 8,           ///< Show planet name if point is in warp well.
        static const int NameNoSpace = 16; // locs_NoSpace = 16           ///< Show nothing at all when in deep space.

        Universe();
        ~Universe();

        int getTurnNumber() const;
        void setTurnNumber(int nr);

        // FIXME: these don't really belong here, but neither does the turn number
        Timestamp getTimestamp() const;
        void setTimestamp(const Timestamp& ts);

        ObjectVector<Ship>& ships();
        const ObjectVector<Ship>& ships() const;

        PlayedShipType& playedShips();

        ObjectVector<Planet>& planets();
        const ObjectVector<Planet>& planets() const;

        PlayedPlanetType& playedPlanets();
        PlayedBaseType& playedBases();

        ObjectVector<IonStorm>& ionStorms();
        const ObjectVector<IonStorm>& ionStorms() const;
        IonStormType& ionStormType();

        MinefieldType& minefields();
        const MinefieldType& minefields() const;

        Configuration& config();
        const Configuration& config() const;

        /** Perform all updates.
            This will poll all updatable objects, and raise the appropriate signals. */
        void notifyListeners();

        /** Mark universe changed. */
        void markChanged();

        void postprocess(PlayerSet_t playingSet, PlayerSet_t availablePlayers, Object::Playability playability,
                         const game::HostVersion& host, const game::config::HostConfiguration& config,
                         afl::string::Translator& tx, afl::sys::LogListener& log);


        /*
         *  Location accessors
         */
        Id_t getPlanetAt(Point pt);
        Id_t getPlanetAt(Point pt,
                         bool gravityFlag,
                         const game::config::HostConfiguration& config,
                         const HostVersion& host);
        Id_t getGravityPlanetAt(Point pt,
                                const game::config::HostConfiguration& config,
                                const HostVersion& host);

        Id_t getAnyShipAt(Point pt);

        String_t getLocationName(Point pt, int flags,
                                 const game::config::HostConfiguration& config,
                                 const HostVersion& host,
                                 afl::string::Translator& tx,
                                 InterpreterInterface& iface);

        afl::base::Signal<void()> sig_preUpdate;
        afl::base::Signal<void()> sig_universeChange;
        
     private:
        Configuration m_config;
        ObjectVector<Ship> m_ships;
        ObjectVector<Planet> m_planets;
        ObjectVector<IonStorm> m_ionStorms;
        std::auto_ptr<MinefieldType> m_minefields;

        bool m_universeChanged;

        int m_turnNumber;
        Timestamp m_timestamp;

        std::auto_ptr<PlayedShipType> m_playedShips;
        std::auto_ptr<PlayedPlanetType> m_playedPlanets;
        std::auto_ptr<PlayedBaseType> m_playedBases;
        std::auto_ptr<IonStormType> m_ionStormType;

// class GUniverse {
//  public:
//     // Initialisation
//     void recomputeOrbitFlags();

//     GDrawingContainer& getDrawings()
//         { return drawings; }
//     GDrawingContainer const& getDrawings() const
//         { return drawings; }

//     GExplosionContainer& getExplosions()
//         { return explosions; }
//     GExplosionContainer const& getExplosions() const
//         { return explosions; }

//     // Location accessors
//     bool      isShipTowed(int sid) const;

//     // Updates

//     void addMessageInformation(const GMessageInformation& msgi);


//     GPlayedShipType    ty_played_ships;
//     GAnyShipType       ty_any_ships;
//     GHistoryShipType   ty_history_ships;
//     GPlayedPlanetType  ty_played_planets;
//     GAnyPlanetType     ty_any_planets;
//     GPlayedBaseType    ty_played_bases;
//     GFleetType         ty_fleets;
//     GUfoType           ty_ufos;
//     GMinefieldType     ty_minefields;

//  private:
//     void init();

//     GPlayerSet   playing_set, data_set;
//     ptr_vector<GIonStorm> storms;
//     GDrawingContainer     drawings;
//     GExplosionContainer   explosions;



    };

} }

#endif
