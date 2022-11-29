/**
  *  \file game/interface/shiptaskpredictor.cpp
  *  \brief Class game::interface::ShipTaskPredictor
  */

#include "game/interface/shiptaskpredictor.hpp"
#include "game/limits.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "interpreter/arguments.hpp"

game::interface::ShipTaskPredictor::ShipTaskPredictor(const game::map::Universe& univ, Id_t id,
                                                      const UnitScoreDefinitionList& scoreDefinitions,
                                                      const game::spec::ShipList& shipList,
                                                      const game::map::Configuration& mapConfig,
                                                      const game::config::HostConfiguration& config,
                                                      const HostVersion& hostVersion,
                                                      const RegistrationKey& key)
    : m_predictor(univ, id, scoreDefinitions, shipList, mapConfig, config, hostVersion, key),
      m_universe(univ),
      m_shipList(shipList),
      m_mapConfig(mapConfig),
      m_config(config),
      m_numPositions(0),
      m_numFuelPositions(0),
      m_numFuelTurns(0),
      m_haveFuel(true)
{
    // ex IntShipPredictor::IntShipPredictor
    m_predictor.addTowee();
}

game::interface::ShipTaskPredictor::~ShipTaskPredictor()
{ }

size_t
game::interface::ShipTaskPredictor::getNumPositions() const
{
    // ex IntShipPredictor::getNumPositions
    return m_numPositions;
}

size_t
game::interface::ShipTaskPredictor::getNumFuelPositions() const
{
    // ex IntShipPredictor::getNumFuelPositions
    return m_numFuelPositions;
}

int
game::interface::ShipTaskPredictor::getNumTurns() const
{
    // ex IntShipPredictor::getNumTurns
    return m_predictor.getNumTurns();
}

int
game::interface::ShipTaskPredictor::getNumFuelTurns() const
{
    // ex IntShipPredictor::getNumFuelTurns
    return m_numFuelTurns;
}

int
game::interface::ShipTaskPredictor::getMovementFuel() const
{
    // ex IntShipPredictor::getMovementFuel
    return m_predictor.getMovementFuelUsed();
}

int
game::interface::ShipTaskPredictor::getCloakFuel() const
{
    // ex IntShipPredictor::getCloakFuel
    return m_predictor.getCloakFuelUsed();
}

int
game::interface::ShipTaskPredictor::getRemainingFuel() const
{
    // ex IntShipPredictor::getRemainingFuel
    return m_predictor.getCargo(Element::Neutronium);
}

int
game::interface::ShipTaskPredictor::getMission() const
{
    return m_predictor.getMission();
}

String_t
game::interface::ShipTaskPredictor::getFriendlyCode() const
{
    // ex IntShipPredictor::getFCode
    return m_predictor.getFriendlyCode();
}

int
game::interface::ShipTaskPredictor::getWarpFactor() const
{
    // ex IntShipPredictor::getSpeed
    return m_predictor.getWarpFactor();
}

bool
game::interface::ShipTaskPredictor::isHyperdriving() const
{
    return m_predictor.isHyperdriving();
}

game::map::Point
game::interface::ShipTaskPredictor::getPosition(size_t index) const
{
    // ex IntShipPredictor::getPosition
    return index < MAX_XYS ? m_positions[index] : game::map::Point();
}

game::map::Point
game::interface::ShipTaskPredictor::getPosition() const
{
    // ex IntShipPredictor::getPosition
    return m_predictor.getPosition();
}

void
game::interface::ShipTaskPredictor::advanceTurn()
{
    // ex IntShipPredictor::predictTurn
    m_predictor.computeTurn();

    // Remember ship position
    if (m_numPositions < MAX_XYS) {
        game::map::Point pt = m_predictor.getPosition();
        if (m_numPositions == 0 || pt != m_positions[m_numPositions-1]) {
            m_positions[m_numPositions++] = pt;
        }
    }

    // Remember that we had fuel
    if (m_haveFuel && m_predictor.getCargo(Element::Neutronium) >= 0) {
        m_numFuelTurns = m_predictor.getNumTurns();
        m_numFuelPositions = m_numPositions;
    }

    // Avoid running out of fuel
    if (m_predictor.getCargo(Element::Neutronium) < 0) {
        m_predictor.setFuel(0);
        m_haveFuel = false;
    }
}

// TaskPredictor:
bool
game::interface::ShipTaskPredictor::predictInstruction(const String_t& name, interpreter::Arguments& args)
{
    // ex IntShipPredictor::predictInstruction, shipint.pas:ShipPredictor
    if (name == "MOVETO") {
        setWaypoint(args);
        int n = 0;
        while (n < game::map::ShipPredictor::MOVEMENT_TIME_LIMIT && !m_predictor.isAtWaypoint()) {
            advanceTurn();
            ++n;
        }
        return true;
    } else if (name == "SETWAYPOINT") {
        setWaypoint(args);
        return true;
    } else if (name == "MOVETOWARDS") {
        setWaypoint(args);
        advanceTurn();
        return true;
    } else if (name == "WAITONETURN") {
        advanceTurn();
        return true;
    } else if (name == "SETSPEED") {
        args.checkArgumentCount(1);
        int32_t speed = 0;
        interpreter::checkIntegerArg(speed, args.getNext(), 0, game::spec::Engine::MAX_WARP);
        m_predictor.setWarpFactor(speed);
        return true;
    } else if (name == "SETFCODE") {
        args.checkArgumentCount(1);
        String_t friendlyCode;
        interpreter::checkStringArg(friendlyCode, args.getNext());
        m_predictor.setFriendlyCode(friendlyCode);
        return true;
    } else if (name == "SETMISSION") {
        args.checkArgumentCount(1, 3);
        int32_t m = 0, i = 0, t = 0;
        interpreter::checkIntegerArg(m, args.getNext(), 0, MAX_NUMBER);
        interpreter::checkIntegerArg(i, args.getNext(), 0, MAX_NUMBER);
        interpreter::checkIntegerArg(t, args.getNext(), 0, MAX_NUMBER);
        m_predictor.setMission(m, i, t);

        const game::spec::Mission* msn = m_shipList.missions().getMissionByNumber(m, PlayerSet_t(m_config.getPlayerMissionNumber(m_predictor.getRealOwner())));
        if (msn != 0 && msn->hasFlag(game::spec::Mission::WaypointMission)) {
            const game::map::Ship* sh = m_universe.ships().get(i);
            game::map::Point shipPos;
            if (sh != 0 && sh->getPosition(shipPos)) {
                // FIXME: handle THost where intercept does not cross the seam?
                m_predictor.setWaypoint(m_mapConfig.getSimpleNearestAlias(shipPos, m_predictor.getPosition()));
            }
        }
        return true;
    } else {
        return true;
    }
}

/** Handle "SetWaypoint"-style command.
    \param args Args passed to command. */
void
game::interface::ShipTaskPredictor::setWaypoint(interpreter::Arguments& args)
{
    // ex IntShipPredictor::setWaypoint
    // Read args
    int32_t x = 2000, y = 2000;
    args.checkArgumentCount(2);
    interpreter::checkIntegerArg(x, args.getNext(), 0, MAX_NUMBER);
    interpreter::checkIntegerArg(y, args.getNext(), 0, MAX_NUMBER);

    // Set waypoint
    m_predictor.setWaypoint(game::map::Point(x, y));
}
