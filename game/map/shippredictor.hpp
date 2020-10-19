/**
  *  \file game/map/shippredictor.hpp
  */
#ifndef C2NG_GAME_MAP_SHIPPREDICTOR_HPP
#define C2NG_GAME_MAP_SHIPPREDICTOR_HPP

#include "game/types.hpp"
#include "game/map/shipdata.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/registrationkey.hpp"
#include "game/element.hpp"
#include "afl/bits/smallset.hpp"

namespace game { namespace map {

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
            UsedAlchemy
        };

        /** Set of properties used in prediction. */
        typedef afl::bits::SmallSet<UsedProperty> UsedProperties_t;

        /** Single ship predictor.
            \param univ              Universe
            \param id                Ship Id
            \param scoreDefinitions  Unit score definitions (required for experience levels)
            \param shipList          Ship list (required for hull/beam/torp/engine specs)
            \param config            Host configuration
            \param hostVersion       Host version
            \param key               Registration key */
        ShipPredictor(const Universe& univ, Id_t id,
                      const UnitScoreDefinitionList& scoreDefinitions,
                      const game::spec::ShipList& shipList,
                      const game::config::HostConfiguration& config,
                      const HostVersion& hostVersion,
                      const RegistrationKey& key);

        /** Tow-pair predictor.
            \param univ              Universe
            \param id                Ship Id
            \param towee             Towee's predictor
            \param scoreDefinitions  Unit score definitions (required for experience levels)
            \param shipList          Ship list (required for hull/beam/torp/engine specs)
            \param config            Host configuration
            \param hostVersion       Host version
            \param key               Registration key */
        ShipPredictor(const Universe& univ, Id_t id, ShipPredictor& towee,
                      const UnitScoreDefinitionList& scoreDefinitions,
                      const game::spec::ShipList& shipList,
                      const game::config::HostConfiguration& config,
                      const HostVersion& hostVersion,
                      const RegistrationKey& key);

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

        /** Get ship's real owner.
            \return real owner */
        int getRealOwner() const;

        /** Get computed friendly code.
            \return friendly code */
        String_t getFriendlyCode() const;

        /** Get universe used for predicting.
            \return universe */
        const Universe& getUniverse() const;

        static const int MOVEMENT_TIME_LIMIT = 30;

     private:
        void init();

        const UnitScoreDefinitionList& m_scoreDefinitions;
        const game::spec::ShipList& m_shipList;
        const game::config::HostConfiguration& m_hostConfiguration;
        const HostVersion& m_hostVersion;
        const RegistrationKey& m_key;

        const Id_t           m_shipId;
        ShipData             m_ship;
        bool                 m_valid;
        ShipPredictor*       m_pTowee;
        const Universe&      m_universe;
        int32_t              m_movementFuelUsed;
        int32_t              m_cloakFuelUsed;
        int                  m_numTurns;
        UsedProperties_t     m_usedProperties;
    };

} }

#endif
