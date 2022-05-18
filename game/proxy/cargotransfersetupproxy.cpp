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
    : m_status(), m_gameSender(gameSender)
{ }

void
game::proxy::CargoTransferSetupProxy::createPlanetShip(WaitIndicator& link, int planetId, int shipId)
{
    // Task
    class Task : public util::Request<Session> {
     public:
        Task(Status& result, int planetId, int shipId)
            : m_result(result), m_planetId(planetId), m_shipId(shipId)
            { }

        void handle(Session& session)
            {
                m_result.setup = CargoTransferSetup::fromPlanetShip(mustHaveGame(session).currentTurn().universe(), m_planetId, m_shipId);
                checkConflict(session, m_result);
            }

     private:
        Status& m_result;
        int m_planetId;
        int m_shipId;
    };

    // Call it
    Task t(m_status, planetId, shipId);
    link.call(m_gameSender, t);
}

void
game::proxy::CargoTransferSetupProxy::createShipShip(WaitIndicator& link, int leftId, int rightId)
{
    // Task
    class Task : public util::Request<Session> {
     public:
        Task(Status& result, int leftId, int rightId)
            : m_result(result), m_leftId(leftId), m_rightId(rightId)
            { }

        void handle(Session& session)
            {
                m_result.setup = CargoTransferSetup::fromShipShip(mustHaveGame(session).currentTurn().universe(), m_leftId, m_rightId);
                checkConflict(session, m_result);
            }

     private:
        Status& m_result;
        int m_leftId;
        int m_rightId;
    };

    // Call it
    Task t(m_status, leftId, rightId);
    link.call(m_gameSender, t);
}

void
game::proxy::CargoTransferSetupProxy::createShipJettison(WaitIndicator& link, int shipId)
{
    // Task
    class Task : public util::Request<Session> {
     public:
        Task(Status& result, int shipId)
            : m_result(result), m_shipId(shipId)
            { }

        void handle(Session& session)
            {
                Game& g = mustHaveGame(session);
                m_result.setup = CargoTransferSetup::fromShipJettison(g.currentTurn().universe(), m_shipId);
                checkConflict(session, m_result);
            }

     private:
        Status& m_result;
        int m_shipId;
    };

    // Call it
    Task t(m_status, shipId);
    link.call(m_gameSender, t);
}

void
game::proxy::CargoTransferSetupProxy::createShipBeamUp(WaitIndicator& link, int shipId)
{
    // Task
    class Task : public util::Request<Session> {
     public:
        Task(Status& result, int shipId)
            : m_result(result), m_shipId(shipId)
            { }

        void handle(Session& session)
            {
                Game& g = mustHaveGame(session);
                m_result.setup = CargoTransferSetup::fromShipBeamUp(g.currentTurn(), m_shipId, mustHaveRoot(session).hostConfiguration());
                checkConflict(session, m_result);
            }

     private:
        Status& m_result;
        int m_shipId;
    };

    // Call it
    Task t(m_status, shipId);
    link.call(m_gameSender, t);
}

const game::proxy::CargoTransferSetupProxy::ConflictInfo*
game::proxy::CargoTransferSetupProxy::getConflictInfo() const
{
    return m_status.conflict.fromId != 0
        ? &m_status.conflict
        : 0;
}

game::actions::CargoTransferSetup
game::proxy::CargoTransferSetupProxy::get() const
{
    return m_status.setup;
}

void
game::proxy::CargoTransferSetupProxy::swapSides()
{
    m_status.setup.swapSides();
}

void
game::proxy::CargoTransferSetupProxy::cancelConflictingTransfer(WaitIndicator& link)
{
    // Task
    class Task : public util::Request<Session> {
     public:
        Task(Status& result)
            : m_result(result)
            { }

        void handle(Session& session)
            {
                m_result.setup.cancelConflictingTransfer(mustHaveGame(session).currentTurn().universe(), m_result.conflict.fromId);
                checkConflict(session, m_result);
                session.notifyListeners();
            }

     private:
        Status& m_result;
    };

    // Call it
    Task t(m_status);
    link.call(m_gameSender, t);
}

void
game::proxy::CargoTransferSetupProxy::checkConflict(Session& s, Status& st)
{
    const game::map::Universe& univ = mustHaveGame(s).currentTurn().universe();

    st.conflict.fromId = st.setup.getConflictingTransferShipId(univ);
    st.conflict.fromName = String_t();
    st.conflict.toId = 0;
    st.conflict.toName = String_t();
    if (const game::map::Ship* fromShip = univ.ships().get(st.conflict.fromId)) {
        st.conflict.fromName = fromShip->getName();
        st.conflict.toId = fromShip->getTransporterTargetId(game::map::Ship::TransferTransporter).orElse(0);
        if (const game::map::Ship* toShip = univ.ships().get(st.conflict.toId)) {
            st.conflict.toName = toShip->getName();
        }
    }
}
