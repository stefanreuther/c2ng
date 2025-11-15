/**
  *  \file game/interface/shiptaskpredictor.hpp
  *  \brief Class game::interface::ShipTaskPredictor
  */
#ifndef C2NG_GAME_INTERFACE_SHIPTASKPREDICTOR_HPP
#define C2NG_GAME_INTERFACE_SHIPTASKPREDICTOR_HPP

#include "game/map/configuration.hpp"
#include "game/map/point.hpp"
#include "game/map/shippredictor.hpp"
#include "interpreter/taskpredictor.hpp"

namespace game { namespace interface {

    /** Predictor for Ship Auto Tasks.
        Predicts movement and fuel usage. */
    class ShipTaskPredictor : public interpreter::TaskPredictor {
     public:
        /** Movement mode. */
        enum MovementMode {
            NormalMovement,     ///< Normal (turn-by-turn) move, precise fuel computation.
            SimpleMovement      ///< Waypoints only, no fuel consumption.
        };

        /** Constructor.
            \param univ              Universe
            \param id                Ship Id
            \param scoreDefinitions  Unit score definitions (required for experience levels)
            \param shipList          Ship list (required for hull/beam/torp/engine specs)
            \param mapConfig         Map configuration
            \param config            Host configuration
            \param hostVersion       Host version
            \param key               Registration key */
        ShipTaskPredictor(const game::map::Universe& univ, Id_t id,
                          const UnitScoreDefinitionList& scoreDefinitions,
                          const game::spec::ShipList& shipList,
                          const game::map::Configuration& mapConfig,
                          const game::config::HostConfiguration& config,
                          const HostVersion& hostVersion,
                          const RegistrationKey& key);
        ~ShipTaskPredictor();

        /** Set movement computation mode.
            \param mode New mode */
        void setMovementMode(MovementMode m);

        /** Get number of computed positions.
            A position is only recorded when the ship moves, so there can be fewer positions than turns.
            Use this to retrieve positions.
            \return number */
        size_t getNumPositions() const;

        /** Get number of positions where this ship had fuel.
            Use this to retrieve positions.
            \return number */
        size_t getNumFuelPositions() const;

        /** Get number of turns computed.
            Use this to display the user.
            \return number */
        int getNumTurns() const;

        /** Get number of turns where this ship had fuel.
            Use this to display the user.
            \return number */
        int getNumFuelTurns() const;

        /** Get amount of fuel used for movement.
            \return fuel */
        int getMovementFuel() const;

        /** Get amount of fuel used for cloaking.
            \return fuel */
        int getCloakFuel() const;

        /** Get remaining amount of fuel.
            \return fuel */
        int getRemainingFuel() const;

        /** Get final mission.
            \return mission number */
        int getMission() const;

        /** Get final friendly code.
            \return friendly code */
        String_t getFriendlyCode() const;

        /** Get final warp factor.
            \return warp factor */
        int getWarpFactor() const;

        /** Check for hyperdrive.
            \return true if ship is hyperdriving */
        bool isHyperdriving() const;

        /** Get position.
            \param index Index [0,getNumPositions())
            \return position */
        game::map::Point getPosition(size_t index) const;

        /** Get final position.
            \return position */
        game::map::Point getPosition() const;

        /** Advance time by one turn. */
        void advanceTurn();

        // TaskPredictor:
        virtual bool predictInstruction(const String_t& name, interpreter::Arguments& args);

     private:
        game::map::ShipPredictor m_predictor;
        const game::map::Universe& m_universe;
        const game::spec::ShipList& m_shipList;
        const game::map::Configuration& m_mapConfig;
        const game::config::HostConfiguration& m_config;
        const HostVersion& m_hostVersion;
        MovementMode m_mode;

        enum { MAX_XYS = 30 };
        game::map::Point m_positions[MAX_XYS];     // ex positions
        size_t m_numPositions;                     // ex num_xys
        size_t m_numFuelPositions;                 // ex fuel_xys
        int m_numFuelTurns;                        // ex turn_fuel
        bool m_haveFuel;                           // ex have_fuel

        void storePosition();
        void setWaypoint(interpreter::Arguments& args);
    };

} }

inline void
game::interface::ShipTaskPredictor::setMovementMode(MovementMode m)
{
    m_mode = m;
}

inline size_t
game::interface::ShipTaskPredictor::getNumPositions() const
{
    // ex IntShipPredictor::getNumPositions
    return m_numPositions;
}

inline size_t
game::interface::ShipTaskPredictor::getNumFuelPositions() const
{
    // ex IntShipPredictor::getNumFuelPositions
    return m_numFuelPositions;
}

inline int
game::interface::ShipTaskPredictor::getNumTurns() const
{
    // ex IntShipPredictor::getNumTurns
    return m_predictor.getNumTurns();
}

inline int
game::interface::ShipTaskPredictor::getNumFuelTurns() const
{
    // ex IntShipPredictor::getNumFuelTurns
    return m_numFuelTurns;
}

inline int
game::interface::ShipTaskPredictor::getMovementFuel() const
{
    // ex IntShipPredictor::getMovementFuel
    return m_predictor.getMovementFuelUsed();
}

inline int
game::interface::ShipTaskPredictor::getCloakFuel() const
{
    // ex IntShipPredictor::getCloakFuel
    return m_predictor.getCloakFuelUsed();
}

inline int
game::interface::ShipTaskPredictor::getRemainingFuel() const
{
    // ex IntShipPredictor::getRemainingFuel
    return m_predictor.getCargo(Element::Neutronium);
}

inline int
game::interface::ShipTaskPredictor::getMission() const
{
    return m_predictor.getMission();
}

inline String_t
game::interface::ShipTaskPredictor::getFriendlyCode() const
{
    // ex IntShipPredictor::getFCode
    return m_predictor.getFriendlyCode();
}

inline int
game::interface::ShipTaskPredictor::getWarpFactor() const
{
    // ex IntShipPredictor::getSpeed
    return m_predictor.getWarpFactor();
}

inline bool
game::interface::ShipTaskPredictor::isHyperdriving() const
{
    // ex IntShipPredictor::isHyperdriving
    return m_predictor.isHyperdriving();
}

inline game::map::Point
game::interface::ShipTaskPredictor::getPosition() const
{
    // ex IntShipPredictor::getPosition
    return m_predictor.getPosition();
}

#endif
