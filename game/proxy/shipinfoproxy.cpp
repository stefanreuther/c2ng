/**
  *  \file game/proxy/shipinfoproxy.cpp
  *  \brief Class game::proxy::ShipInfoProxy
  */

#include "game/proxy/shipinfoproxy.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/turn.hpp"

using game::map::Ship;
using game::spec::ShipList;
using util::NumberFormatter;

game::proxy::ShipInfoProxy::ShipInfoProxy(const util::RequestSender<Session>& gameSender)
    : m_gameSender(gameSender)
{ }

game::proxy::ShipInfoProxy::CargoStatus
game::proxy::ShipInfoProxy::getCargo(WaitIndicator& ind, Id_t shipId, int which, game::map::ShipCargoInfos_t& out)
{
    class Task : public util::Request<Session> {
     public:
        Task(Id_t shipId, int which)
            : m_shipId(shipId), m_which(which), m_result(NoCargo), m_out()
            { }
        virtual void handle(Session& session)
            {
                const Root* pRoot = session.getRoot().get();
                const Game* pGame = session.getGame().get();
                const ShipList* pList =  session.getShipList().get();
                if (pRoot != 0 && pGame != 0 && pList != 0) {
                    const Turn& turn = pGame->viewpointTurn();
                    if (const Ship* ship = turn.universe().ships().get(m_shipId)) {
                        if (ship->isPlayable(Ship::Playable)) {
                            m_result = CurrentShip;
                        } else {
                            NumberFormatter fmt = pRoot->userConfiguration().getNumberFormatter();
                            if ((m_which & GetLastKnownCargo) != 0) {
                                packShipLastKnownCargo(m_out, *ship, turn.getTurnNumber(), fmt, *pList, session.translator());
                            }
                            if ((m_which & GetMassRanges) != 0) {
                                packShipMassRanges    (m_out, *ship,                       fmt, *pList, session.translator());
                            }
                            m_result = HistoryCargo;
                        }
                    }
                }
            }
        CargoStatus getResult() const
            { return m_result; }
        const game::map::ShipCargoInfos_t& getCargoInformation() const
            { return m_out; }
     private:
        const Id_t m_shipId;
        const int m_which;
        CargoStatus m_result;
        game::map::ShipCargoInfos_t m_out;
    };

    Task t(shipId, which);
    ind.call(m_gameSender, t);
    out = t.getCargoInformation();         // Copy to avoid aliasing
    return t.getResult();
}

game::map::ShipExperienceInfo
game::proxy::ShipInfoProxy::getExperienceInfo(WaitIndicator& ind, Id_t shipId)
{
    class Task : public util::Request<Session> {
     public:
        Task(Id_t shipId)
            : m_shipId(shipId), m_result()
            { }
        virtual void handle(Session& session)
            {
                const Root* pRoot = session.getRoot().get();
                const Game* pGame = session.getGame().get();
                const ShipList* pList =  session.getShipList().get();
                if (pRoot != 0 && pGame != 0 && pList != 0) {
                    const Turn& turn = pGame->viewpointTurn();
                    if (const Ship* ship = turn.universe().ships().get(m_shipId)) {
                        m_result = game::map::packShipExperienceInfo(*ship, pGame->shipScores(), pRoot->hostConfiguration(), pRoot->hostVersion(), *pList);
                    }
                }
            }
        const game::map::ShipExperienceInfo& getResult() const
            { return m_result; }
     private:
        const Id_t m_shipId;
        game::map::ShipExperienceInfo m_result;
    };

    Task t(shipId);
    ind.call(m_gameSender, t);
    return t.getResult();
}
