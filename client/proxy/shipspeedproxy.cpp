/**
  *  \file client/proxy/shipspeedproxy.cpp
  *  \brief Class client::proxy::ShipSpeedProxy
  */

#include "client/proxy/shipspeedproxy.hpp"
#include "game/actions/changeshipfriendlycode.hpp"
#include "game/game.hpp"
#include "game/map/fleet.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "game/map/ship.hpp"

using afl::base::Ptr;
using game::Id_t;
using game::actions::ChangeShipFriendlyCode;
using game::map::Fleet;
using game::map::FleetMember;
using game::map::Ship;

/*
 *  Magic numbers
 *
 *  Widgets don't know about warp factor range, so we provide them.
 */

namespace {
    const int MAX_WARP = 9;
    const int HYPER_WARP = MAX_WARP+1;
    const int WARP_FOR_HYP = 2;
}

/*
 *  Trampoline
 *
 *  We need a Fleet/FleetMember to access the ship's fleet status.
 *  In particular, we need to know whether the *fleet* can hyperjump, not just the ship.
 *
 *  If the fleet can hyperjump, we also need a ChangeShipFriendlyCode action
 *  to control the fleet's friendly codes.
 */

class client::proxy::ShipSpeedProxy::Trampoline : public util::SlaveObject<game::Session> {
 public:
    Trampoline(Id_t shipId)
        : m_shipId(shipId),
          m_pTurn(),
          m_pShipList(),
          m_pRoot(),
          m_pShip(),
          m_pFriendlyCodeChanger(),
          m_status()
        { }

    void init(game::Session& session)
        {
            // Default values
            m_status.currentSpeed = 0;
            m_status.maxSpeed = 0;
            m_status.hyperSpeedMarker = HYPER_WARP;

            // Determine preconditions
            game::Root* pRoot = session.getRoot().get();
            game::Game* pGame = session.getGame().get();
            game::spec::ShipList* pShipList = session.getShipList().get();
            if (pRoot != 0 && pGame != 0 && pShipList != 0) {
                m_pTurn = &pGame->currentTurn();
                m_pShipList = pShipList;
                m_pRoot = pRoot;
                m_pShip = m_pTurn->universe().ships().get(m_shipId);
                if (m_pShip != 0) {
                    // Default to normal ship
                    m_status.maxSpeed = MAX_WARP;
                    m_status.currentSpeed = m_pShip->getWarpFactor().orElse(0);

                    // Hyperdrive capable?
                    Fleet f(m_pTurn->universe(), *m_pShip);
                    if (f.hasSpecialFunction(game::spec::HullFunction::Hyperdrive, pGame->shipScores(), *pShipList, pRoot->hostConfiguration())) {
                        // OK, fleet can hyperjump. Limit is 10.
                        m_status.maxSpeed = HYPER_WARP;
                        if (m_pShip->isHyperdriving(pGame->shipScores(), *pShipList, pRoot->hostConfiguration())) {
                            m_status.currentSpeed = HYPER_WARP;
                        }

                        // Also set up a friendly code changer
                        m_pFriendlyCodeChanger.reset(new ChangeShipFriendlyCode(m_pTurn->universe()));
                        m_pFriendlyCodeChanger->addFleet(m_shipId, pShipList->friendlyCodes(), session.rng());
                    }
                }
            }
        }

    void done(game::Session& /*session*/)
        { }

    const Status& getStatus() const
        { return m_status; }

    void setSpeed(int n)
        {
            // ex WShipSpeedSelector::widgetUpdated [sort-of]
            if (m_pTurn.get() != 0 && m_pShipList.get() != 0 && m_pRoot.get() != 0 && m_pShip != 0 && n >= 0 && n <= m_status.maxSpeed) {
                FleetMember fm(m_pTurn->universe(), *m_pShip);
                if (n == HYPER_WARP) {
                    if (m_pFriendlyCodeChanger.get() != 0) {
                        m_pFriendlyCodeChanger->setFriendlyCode("HYP");
                    }
                    fm.setWarpFactor(WARP_FOR_HYP, m_pRoot->hostConfiguration(), *m_pShipList);
                } else {
                    if (m_pFriendlyCodeChanger.get() != 0) {
                        m_pFriendlyCodeChanger->unsetFriendlyCode("HYP");
                    }
                    fm.setWarpFactor(n, m_pRoot->hostConfiguration(), *m_pShipList);
                }
                m_status.currentSpeed = n;
                m_pTurn->notifyListeners();
            }
        }

 private:
    Id_t m_shipId;
    Ptr<game::Turn> m_pTurn;
    Ptr<game::spec::ShipList> m_pShipList;
    Ptr<game::Root> m_pRoot;
    Ship* m_pShip;
    std::auto_ptr<ChangeShipFriendlyCode> m_pFriendlyCodeChanger;
    Status m_status;
};



client::proxy::ShipSpeedProxy::ShipSpeedProxy(util::RequestSender<game::Session> gameSender, game::Id_t shipId)
    : m_trampoline(gameSender, new Trampoline(shipId))
{ }

client::proxy::ShipSpeedProxy::~ShipSpeedProxy()
{ }

client::proxy::ShipSpeedProxy::Status
client::proxy::ShipSpeedProxy::getStatus(Downlink& link)
{
    class InitTask : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        InitTask(Status& result)
            : m_result(result)
            { }
        void handle(game::Session& /*session*/, Trampoline& t)
            { m_result = t.getStatus(); }
     private:
        Status& m_result;
    };

    Status result = {0,0,0};
    InitTask t(result);
    link.call(m_trampoline, t);
    return result;
}

void
client::proxy::ShipSpeedProxy::setSpeed(int n)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(int n)
            : m_n(n)
            { }
        void handle(game::Session& /*session*/, Trampoline& t)
            { t.setSpeed(m_n); }
     private:
        int m_n;
    };
    m_trampoline.postNewRequest(new Task(n));
}
