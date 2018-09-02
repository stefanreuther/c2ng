/**
  *  \file client/proxy/hullspecificationproxy.cpp
  */

#include "client/proxy/hullspecificationproxy.hpp"
#include "game/turn.hpp"
#include "game/spec/shiplist.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "ui/res/resid.hpp"
#include "util/math.hpp"

using game::config::HostConfiguration;

client::proxy::HullSpecificationProxy::HullSpecificationProxy(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& reply)
    : m_gameSender(gameSender),
      m_reply(reply, *this)
{ }

void
client::proxy::HullSpecificationProxy::setExistingShipId(game::Id_t id)
{
    class Query : public util::Request<game::Session> {
     public:
        Query(game::Id_t id, util::RequestSender<HullSpecificationProxy> reply)
            : m_id(id),
              m_reply(reply)
            { }
        void handle(game::Session& s)
            {
                game::ShipQuery q;
                const game::Game* pGame = s.getGame().get();
                const game::spec::ShipList* pShipList = s.getShipList().get();
                const game::Root* pRoot = s.getRoot().get();
                if (pGame != 0 && pShipList != 0 && pRoot != 0) {
                    const game::Turn* pTurn = pGame->getViewpointTurn().get();
                    if (pTurn != 0) {
                        q.initForExistingShip(pTurn->universe(), m_id, *pShipList, pRoot->hostConfiguration(), pGame->shipScores());
                    }
                    sendReply(q, *pShipList, *pRoot, pGame->getViewpointPlayer(), m_reply);
                }
                // FIXME: should we handle the "no session" case?
            }
     private:
        game::Id_t m_id;
        util::RequestSender<HullSpecificationProxy> m_reply;
    };
    m_gameSender.postNewRequest(new Query(id, m_reply.getSender()));
}

void
client::proxy::HullSpecificationProxy::sendReply(game::ShipQuery& q,
                                                 const game::spec::ShipList& shipList,
                                                 const game::Root& root,
                                                 int player,
                                                 util::RequestSender<HullSpecificationProxy> reply)
{
    HullSpecification result = HullSpecification();
    if (const game::spec::Hull* pHull = shipList.hulls().get(q.getHullType())) {
        const HostConfiguration& config = root.hostConfiguration();

        result.name = pHull->getName(shipList.componentNamer());
        result.image = ui::res::makeResourceId(ui::res::SHIP, pHull->getInternalPictureNumber(), pHull->getId());

        result.hullId = pHull->getId();
        result.mass = pHull->getMass();
        result.numEngines = pHull->getNumEngines();
        result.techLevel = pHull->getTechLevel();
        result.maxCrew = pHull->getMaxCrew();
        result.maxCargo = pHull->getMaxCargo();
        result.maxFuel = pHull->getMaxFuel();
        result.maxBeams = pHull->getMaxBeams();
        result.maxLaunchers = pHull->getMaxLaunchers();
        result.numBays = pHull->getNumBays();

        if (pHull->getMass() == 0) {
            result.mineHitDamage = 100;
        } else if (root.hostVersion().isPHost()) {
            result.mineHitDamage = 100 * config[HostConfiguration::MineHitDamageFor100KT]() / pHull->getMass();
        } else {
            result.mineHitDamage = util::divideAndRound(10000, pHull->getMass() + 1);
        }

        // FIXME:
        // int fuelBurnPerTurn;
        // int fuelBurnPerFight;

        result.cost = pHull->cost();

        if (root.hostVersion().isPBPGame(config)) {
            // int build, kill, scrap;
            int32_t mass = pHull->getMass();
            if (root.hostVersion().isPHost()) {
                /* Build */
                result.pointsToBuild = std::max(mass * config[HostConfiguration::PBPCostPer100KT](player) / 100,
                                                config[HostConfiguration::PBPMinimumCost](player));

                /* Kill, estimation (since there are many ways to destroy it) */
                result.pointsForKilling = mass * (config[HostConfiguration::PALAggressorPointsPer10KT](player) + config[HostConfiguration::PALAggressorKillPointsPer10KT](player)) / 10
                    + config[HostConfiguration::PALCombatAggressor](player);

                /* Scrap */
                result.pointsForScrapping = mass * config[HostConfiguration::PALRecyclingPer10KT](player) / 10;
            } else {
                /* Build:
                   - Vendetta (100 kt) => 2
                   - Loki (101 kt) => 3 */
                result.pointsToBuild = (mass + 49) / 50;

                /* Kill:
                   - Dwarfstar (100 kt) => 2 */
                result.pointsForKilling = (mass / 100) + 1;

                /* Scrap */
                result.pointsForScrapping = 1;
            }
        }

        // Players
        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            if (shipList.hullAssignments().getIndexFromHull(root.hostConfiguration(), i, pHull->getId()) != 0) {
                result.players += i;
            }
        }
    }

    sendReply(result, reply);
}

void
client::proxy::HullSpecificationProxy::sendReply(const HullSpecification& result,
                                                 util::RequestSender<HullSpecificationProxy> reply)
{
    class Reply : public util::Request<HullSpecificationProxy> {
     public:
        Reply(const HullSpecification& result)
            : m_result(result)
            { }
        void handle(HullSpecificationProxy& proxy)
            { proxy.sig_update.raise(m_result); }
     private:
        HullSpecification m_result;
    };
    reply.postNewRequest(new Reply(result));
}
