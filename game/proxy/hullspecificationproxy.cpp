/**
  *  \file game/proxy/hullspecificationproxy.cpp
  *  \brief Class game::proxy::HullSpecificationProxy
  */

#include "game/proxy/hullspecificationproxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "util/math.hpp"

using game::config::HostConfiguration;
using game::spec::info::PictureNamer;
using afl::base::Ptr;

game::proxy::HullSpecificationProxy::HullSpecificationProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
    : m_gameSender(gameSender),
      m_reply(reply, *this),
      m_picNamer(picNamer.release())
{ }

void
game::proxy::HullSpecificationProxy::setExistingShipId(Id_t id)
{
    class Query : public util::Request<Session> {
     public:
        Query(Id_t id, util::RequestSender<HullSpecificationProxy> reply, Ptr<PictureNamer> picNamer)
            : m_id(id),
              m_reply(reply),
              m_picNamer(picNamer)
            { }
        void handle(Session& s)
            {
                ShipQuery q;
                const Game* pGame = s.getGame().get();
                const game::spec::ShipList* pShipList = s.getShipList().get();
                const Root* pRoot = s.getRoot().get();
                if (pGame != 0 && pShipList != 0 && pRoot != 0) {
                    const Turn* pTurn = pGame->getViewpointTurn().get();
                    if (pTurn != 0) {
                        q.initForExistingShip(pTurn->universe(), m_id, *pShipList, pRoot->hostConfiguration(), pGame->shipScores());
                    }
                    sendReply(q, *pShipList, *pRoot, pGame->getViewpointPlayer(), *m_picNamer, m_reply);
                }
                // FIXME: should we handle the "no session" case?
            }
     private:
        Id_t m_id;
        util::RequestSender<HullSpecificationProxy> m_reply;
        Ptr<PictureNamer> m_picNamer;
    };
    m_gameSender.postNewRequest(new Query(id, m_reply.getSender(), m_picNamer));
}

void
game::proxy::HullSpecificationProxy::sendReply(ShipQuery& q,
                                               const game::spec::ShipList& shipList,
                                               const Root& root,
                                               int player,
                                               game::spec::info::PictureNamer& picNamer,
                                               util::RequestSender<HullSpecificationProxy> reply)
{
    HullSpecification result = HullSpecification();
    if (const game::spec::Hull* pHull = shipList.hulls().get(q.getHullType())) {
        const HostConfiguration& config = root.hostConfiguration();

        result.name = pHull->getName(shipList.componentNamer());
        result.image = picNamer.getHullPicture(*pHull);

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

        // FIXME: use Hull::getMineHitDamage()
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

        // FIXME: add as functions of Hull
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
        for (int i = 1; i <= MAX_PLAYERS; ++i) {
            // FIXME: as function of HullAssignmentList?
            if (shipList.hullAssignments().getIndexFromHull(root.hostConfiguration(), i, pHull->getId()) != 0) {
                result.players += i;
            }
        }
    }

    sendReply(result, reply);
}

void
game::proxy::HullSpecificationProxy::sendReply(const HullSpecification& result,
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
