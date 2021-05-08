/**
  *  \file game/proxy/hullspecificationproxy.cpp
  *  \brief Class game::proxy::HullSpecificationProxy
  */

#include "game/proxy/hullspecificationproxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/spec/info/info.hpp"
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
                    sendReply(q, *pShipList, *pRoot, pTurn ? &pTurn->universe() : 0, pGame->getViewpointPlayer(), *m_picNamer, s.translator(), m_reply);
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
                                               const game::map::Universe* pUniv,
                                               int player,
                                               game::spec::info::PictureNamer& picNamer,
                                               afl::string::Translator& tx,
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
            result.pointsToBuild = pHull->getPointsToBuild(player, root.hostVersion(), config);
            result.pointsForKilling = pHull->getPointsForKilling(player, root.hostVersion(), config);
            result.pointsForScrapping = pHull->getPointsForScrapping(player, root.hostVersion(), config);
        }

        // Players
        result.players = shipList.hullAssignments().getPlayersForHull(root.hostConfiguration(), pHull->getId());

        // Abilities
        if (pUniv != 0) {
            game::spec::HullFunctionList hfList;
            q.enumerateShipFunctions(hfList, *pUniv, shipList, root.hostConfiguration(), false);
            hfList.simplify();
            hfList.sortForNewShip(q.getPlayerDisplaySet());

            game::spec::info::describeHullFunctions(result.abilities, hfList, shipList, picNamer, root, tx);
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
