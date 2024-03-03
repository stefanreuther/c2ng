/**
  *  \file game/map/shippredictor.hpp
  *  \brief Class game::map::ShipPredictor
  */
#ifndef C2NG_GAME_MAP_SHIPPREDICTOR_HPP
#define C2NG_GAME_MAP_SHIPPREDICTOR_HPP

#include <memory>
#include "afl/bits/smallset.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/element.hpp"
#include "game/hostversion.hpp"
#include "game/map/shipdata.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/types.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {

    class Configuration;
    class Universe;

    /** Ship Turn Predictor.
        This object manages prediction of a ship's future, especially movement fuel and ETA computation.

        To compute prediction for a single ship, use the constructor taking an Id;
        to predict a tower/towee pair, construct a secondary object for the towee, and pass it in to the tower's predictor.
        The tower's computeTurn() method will then also compute the towee's prediction.

        After constructing, call computeTurn() or computeMovement(), and use getters to obtain results.
        Use setters to override ship properties. */
    class ShipPredictor {
     public:
        /** Property used in prediction. */
        enum UsedProperty {
            UsedMission,
            UsedFCode,
            UsedShipyard,
            UsedRepair,
            UsedCloak,
            UsedDamageLimit,
            UsedTowee,
            UsedAlchemy,
            UsedBuildFighters
        };

        /** Set of properties used in prediction. */
        typedef afl::bits::SmallSet<UsedProperty> UsedProperties_t;

        /** Create ship predictor.
            \param univ              Universe
            \param id                Ship Id
            \param scoreDefinitions  Unit score definitions (required for experience levels)
            \param shipList          Ship list (required for hull/beam/torp/engine specs)
            \param mapConfig         Map configuration
            \param config            Host configuration
            \param hostVersion       Host version
            \param key               Registration key */
        ShipPredictor(const Universe& univ, Id_t id,
                      const UnitScoreDefinitionList& scoreDefinitions,
                      const game::spec::ShipList& shipList,
                      const Configuration& mapConfig,
                      const game::config::HostConfiguration& config,
                      const HostVersion& hostVersion,
                      const RegistrationKey& key);

        /** Add predictor for ship's towee, if any.
            If this ship is towing an applicable ship (with full data), prediction will use live data,
            that is, compute the towee's mission effects.

            If this function is not used, or if the towed ship is not available with full data,
            prediction will use static data.

            This function has no effect if the ship does not actually use a Tow mission. */
        void addTowee();


        /*
         *  Inquiry
         */

        /** Get total fuel used for movement.
            \return fuel usage */
        int32_t getMovementFuelUsed() const;

        /** Get total fuel used for cloaking.
            \return fuel usage */
        int32_t getCloakFuelUsed() const;

        /** Get number of turns computed.
            \return number of turns */
        int getNumTurns() const;

        /** Check whether computation was stopped because the turn limit was exceeded.
            \return true if turn limit exceeded */
        bool isAtTurnLimit() const;

        /** Get set of ship properties used for prediction.
            \return set */
        UsedProperties_t getUsedProperties() const;


        /*
         *  Computation
         */

        /** Compute one turn.
            This will compute all changes for the ship.
            It will also update the towed ship predictor, if any. */
        void computeTurn();

        /** Compute this ship's movement.
            Stops when movement is over or time runs out. */
        void computeMovement();


        /*
         *  Manipulation
         */

        /** Override this ship's position.
            \param pt New position */
        void setPosition(Point pt);

        /** Override this ship's waypoint.
            \param pt New waypoint (absolute position, not relative) */
        void setWaypoint(Point pt);

        /** Override this ship's speed.
            \param warp Speed */
        void setWarpFactor(int warp);

        /** Override this ship's mission.
            \param m  New mission
            \param i  Intercept argument
            \param t  Tow argument */
        void setMission(int m, int i, int t);

        /** Override this ship's friendly code.
            \param s Friendly code */
        void setFriendlyCode(String_t s);

        /** Override this ship's amount of fuel.
            \param fuel Amount */
        void setFuel(int fuel);

        /*
         *  Inquiry
         */

        /** Check whether ship has reached its waypoint.
            \return true if waypoint reached */
        bool isAtWaypoint() const;

        /** Get computed position.
            \return position */
        Point getPosition() const;

        /** Get computed cargo.
            \param el Cargo type
            \return amount */
        int getCargo(Element::Type el) const;

        /** Get computed warp factor.
            \return warp factor */
        int getWarpFactor() const;

        /** Check for hyperdrive.
            \return true if ship is hyperdriving */
        bool isHyperdriving() const;

        /** Get ship's real owner.
            \return real owner */
        int getRealOwner() const;

        /** Get computed current mission.
            \return mission number */
        int getMission() const;

        /** Get computed friendly code.
            \return friendly code */
        String_t getFriendlyCode() const;

        /** Get name of towed ship.
            \return name; empty if none */
        String_t getTowedShipName() const;

        /** Get universe used for predicting.
            \return universe */
        const Universe& getUniverse() const;

        /** Access ship list.
            \return ship list */
        const game::spec::ShipList& shipList() const;

        static const int MOVEMENT_TIME_LIMIT = 30;

     private:
        void init();

        const UnitScoreDefinitionList& m_scoreDefinitions;
        const game::spec::ShipList& m_shipList;
        const Configuration& m_mapConfig;
        const game::config::HostConfiguration& m_hostConfiguration;
        const HostVersion& m_hostVersion;
        const RegistrationKey& m_key;

        const Id_t           m_shipId;
        ShipData             m_ship;
        bool                 m_valid;
        std::auto_ptr<ShipPredictor> m_pTowee;
        const Universe&      m_universe;
        int32_t              m_movementFuelUsed;
        int32_t              m_cloakFuelUsed;
        int                  m_numTurns;
        UsedProperties_t     m_usedProperties;
    };


    /** Compute optimum warp factor.
        \param univ              Universe
        \param shipId            Ship Id
        \param moveFrom          Starting point of movement (ship position)
        \param moveTo            Target of movement (waypoint)
        \param scoreDefinitions  Unit score definitions (required for experience levels)
        \param shipList          Ship list (required for hull/beam/torp/engine specs)
        \param mapConfig         Map configuration
        \param root              Root (provides host configuration, version, key)
        \return optimum warp factor */
    int getOptimumWarp(const Universe& univ, Id_t shipId,
                       Point moveFrom, Point moveTo,
                       const UnitScoreDefinitionList& scoreDefinitions,
                       const game::spec::ShipList& shipList,
                       const Configuration& mapConfig,
                       const Root& root);

    /** Compute ship-independante movement time.
        \param moveFrom          Starting point of movement (ship position)
        \param moveTo            Target of movement (waypoint)
        \param way               Distance covered per turn (warp-squared, times-two for gravitonic)
        \param univ              Universe (for warp wells)
        \param mapConfig         Map configuration
        \param root              Root (for configuration, host version)
        \return Time taken; MOVEMENT_TIME_LIMIT if limit exceeded */
    int computeMovementTime(Point moveFrom, Point moveTo, int way, const Universe& univ, const Configuration& mapConfig, const Root& root);

} }

#endif
