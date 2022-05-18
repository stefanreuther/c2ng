/**
  *  \file game/proxy/shipspeedproxy.cpp
  *  \brief Class game::proxy::ShipSpeedProxy
  */

#include "game/proxy/shipspeedproxy.hpp"
#include "game/actions/changeshipfriendlycode.hpp"
#include "game/game.hpp"
#include "game/map/fleet.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using afl::base::Ptr;
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

class game::proxy::ShipSpeedProxy::Trampoline {
 public:
    Trampoline(Session& session, Id_t shipId)
        : m_shipId(shipId),
          m_pTurn(),
          m_pGame(),
          m_pShipList(),
          m_pRoot(),
          m_pShip(),
          m_pFriendlyCodeChanger(),
          m_status()
        {
            // Default values
            m_status.currentSpeed = 0;
            m_status.maxSpeed = 0;
            m_status.hyperSpeedMarker = HYPER_WARP;

            // Determine preconditions
            Root* pRoot = session.getRoot().get();
            Game* pGame = session.getGame().get();
            game::spec::ShipList* pShipList = session.getShipList().get();
            if (pRoot != 0 && pGame != 0 && pShipList != 0) {
                m_pTurn = &pGame->currentTurn();
                m_pGame = pGame;
                m_pShipList = pShipList;
                m_pRoot = pRoot;
                m_pShip = m_pTurn->universe().ships().get(m_shipId);
                if (m_pShip != 0) {
                    // Default to normal ship
                    m_status.maxSpeed = MAX_WARP;
                    m_status.currentSpeed = m_pShip->getWarpFactor().orElse(0);

                    // Hyperdrive capable?
                    Fleet f(m_pTurn->universe(), *m_pShip);
                    if (f.hasSpecialFunction(game::spec::BasicHullFunction::Hyperdrive, pGame->shipScores(), *pShipList, pRoot->hostConfiguration())) {
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

    const Status& getStatus() const
        { return m_status; }

    void setSpeed(int n)
        {
            // ex WShipSpeedSelector::widgetUpdated [sort-of]
            if (m_pTurn.get() != 0 && m_pShipList.get() != 0 && m_pRoot.get() != 0 && m_pShip != 0 && n >= 0 && n <= m_status.maxSpeed) {
                FleetMember fm(m_pTurn->universe(), *m_pShip, m_pGame->mapConfiguration());
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
    Ptr<Turn> m_pTurn;
    Ptr<Game> m_pGame;
    Ptr<game::spec::ShipList> m_pShipList;
    Ptr<Root> m_pRoot;
    Ship* m_pShip;
    std::auto_ptr<ChangeShipFriendlyCode> m_pFriendlyCodeChanger;
    Status m_status;
};


class game::proxy::ShipSpeedProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(Id_t shipId)
        : m_shipId(shipId)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_shipId); }
 private:
    Id_t m_shipId;
};


game::proxy::ShipSpeedProxy::ShipSpeedProxy(util::RequestSender<Session> gameSender, Id_t shipId)
    : m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(shipId)))
{ }

game::proxy::ShipSpeedProxy::~ShipSpeedProxy()
{ }

game::proxy::ShipSpeedProxy::Status
game::proxy::ShipSpeedProxy::getStatus(WaitIndicator& link)
{
    class InitTask : public util::Request<Trampoline> {
     public:
        InitTask(Status& result)
            : m_result(result)
            { }
        void handle(Trampoline& t)
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
game::proxy::ShipSpeedProxy::setSpeed(int n)
{
    m_trampoline.postRequest(&Trampoline::setSpeed, n);
}
