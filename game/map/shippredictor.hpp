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

namespace game { namespace map {

    class Universe;

    // /** Ship Turn Predictor. This object manages prediction of a ship's
    //     future, especially movement fuel and ETA computation.

    //     To compute prediction for a single ship, use the two-argument
    //     constructor. To predict a tower/towee pair, construct a secondary
    //     object for the towee, and pass it in to the tower's predictor
    //     using the three-argument constructor. The tower's computeTurn()
    //     method will then also compute the towee's prediction. */
    class ShipPredictor {
     public:
        ShipPredictor(const Universe& univ, Id_t id,
                      const UnitScoreDefinitionList& scoreDefinitions,
                      const game::spec::ShipList& shipList,
                      const game::config::HostConfiguration& config,
                      const HostVersion& hostVersion,
                      const RegistrationKey& key);
        ShipPredictor(const Universe& univ, Id_t id, ShipPredictor& towee,
                      const UnitScoreDefinitionList& scoreDefinitions,
                      const game::spec::ShipList& shipList,
                      const game::config::HostConfiguration& config,
                      const HostVersion& hostVersion,
                      const RegistrationKey& key);

        // Fuel usage
        int32_t  getMovementFuelUsed() const;
        int32_t  getCloakFuelUsed() const;

        // Time
        int      getNumTurns() const;
        bool     isAtTurnLimit() const;

        // Computation
        void     computeTurn();
        void     computeMovement();

        // Manipulation
        void     setPosition(Point pt);
        void     setWaypoint(Point pt);
        void     setWarpFactor(int warp);
        void     setEngineType(int engine);    // FIXME: remove, probably not needed
        void     setMission(int m, int i, int t);
        void     setFriendlyCode(String_t s);
        void     setFuel(int fuel);

        bool     isAtWaypoint() const;
        Point    getPosition() const;
        int      getFuel() const;
        int      getCargo(Element::Type el) const;
        int      getWarpFactor() const;
        int      getRealOwner() const;
        String_t getFriendlyCode() const;
        const Universe& getUniverse() const;

        static const int MOVEMENT_TIME_LIMIT = 30;

     private:
        void init();

        const UnitScoreDefinitionList& m_scoreDefinitions;
        const game::spec::ShipList& m_shipList;
        const game::config::HostConfiguration& m_hostConfiguration;
        const HostVersion& m_hostVersion;
        const RegistrationKey& m_key;

        Id_t                 id;
        ShipData             ship;
        bool                 valid;
        ShipPredictor*       towee_override;
        const Universe&      univ;
        int32_t              movement_fuel_used;
        int32_t              cloak_fuel_used;
        int                  turns_computed;
    };

} }

#endif
