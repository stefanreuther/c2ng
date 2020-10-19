/**
  *  \file game/proxy/cargotransfersetupproxy.cpp
  *  \brief Class game::proxy::CargoTransferSetupProxy
  */

#include "game/proxy/cargotransfersetupproxy.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using game::actions::CargoTransferSetup;
using game::actions::mustHaveGame;
using game::actions::mustHaveRoot;

game::proxy::CargoTransferSetupProxy::CargoTransferSetupProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

game::actions::CargoTransferSetup
game::proxy::CargoTransferSetupProxy::createPlanetShip(WaitIndicator& link, int planetId, int shipId)
{
    // Task
    class Task : public util::Request<Session> {
     public:
        Task(CargoTransferSetup& result, int planetId, int shipId)
            : m_result(result), m_planetId(planetId), m_shipId(shipId)
            { }

        void handle(Session& session)
            { m_result = CargoTransferSetup::fromPlanetShip(mustHaveGame(session).currentTurn().universe(), m_planetId, m_shipId); }

     private:
        CargoTransferSetup& m_result;
        int m_planetId;
        int m_shipId;
    };

    // Call it
    CargoTransferSetup result;
    Task t(result, planetId, shipId);
    link.call(m_gameSender, t);
    return result;
}

game::actions::CargoTransferSetup
game::proxy::CargoTransferSetupProxy::createShipShip(WaitIndicator& link, int leftId, int rightId)
{
    // Task
    class Task : public util::Request<Session> {
     public:
        Task(CargoTransferSetup& result, int leftId, int rightId)
            : m_result(result), m_leftId(leftId), m_rightId(rightId)
            { }

        void handle(Session& session)
            { m_result = CargoTransferSetup::fromShipShip(mustHaveGame(session).currentTurn().universe(), m_leftId, m_rightId); }

     private:
        CargoTransferSetup& m_result;
        int m_leftId;
        int m_rightId;
    };

    // Call it
    CargoTransferSetup result;
    Task t(result, leftId, rightId);
    link.call(m_gameSender, t);
    return result;
}

game::actions::CargoTransferSetup
game::proxy::CargoTransferSetupProxy::createShipJettison(WaitIndicator& link, int shipId)
{
    // Task
    class Task : public util::Request<Session> {
     public:
        Task(CargoTransferSetup& result, int shipId)
            : m_result(result), m_shipId(shipId)
            { }

        void handle(Session& session)
            { m_result = CargoTransferSetup::fromShipJettison(mustHaveGame(session).currentTurn().universe(), m_shipId); }

     private:
        CargoTransferSetup& m_result;
        int m_shipId;
    };

    // Call it
    CargoTransferSetup result;
    Task t(result, shipId);
    link.call(m_gameSender, t);
    return result;
}

game::actions::CargoTransferSetup
game::proxy::CargoTransferSetupProxy::createShipBeamUp(WaitIndicator& link, int shipId)
{
    // Task
    class Task : public util::Request<Session> {
     public:
        Task(CargoTransferSetup& result, int shipId)
            : m_result(result), m_shipId(shipId)
            { }

        void handle(Session& session)
            { m_result = CargoTransferSetup::fromShipBeamUp(mustHaveGame(session).currentTurn(), m_shipId, mustHaveRoot(session).hostConfiguration()); }

     private:
        CargoTransferSetup& m_result;
        int m_shipId;
    };

    // Call it
    CargoTransferSetup result;
    Task t(result, shipId);
    link.call(m_gameSender, t);
    return result;
}
