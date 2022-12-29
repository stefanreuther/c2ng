/**
  *  \file game/proxy/hullspecificationproxy.cpp
  *  \brief Class game::proxy::HullSpecificationProxy
  */

#include "game/proxy/hullspecificationproxy.hpp"
#include "game/game.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/root.hpp"
#include "game/score/compoundscore.hpp"
#include "game/shipquery.hpp"
#include "game/spec/info/info.hpp"
#include "game/spec/info/nullpicturenamer.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"

using game::config::HostConfiguration;
using game::spec::ShipList;
using game::spec::info::PictureNamer;

/*
 *  Trampoline
 */

class game::proxy::HullSpecificationProxy::Trampoline {
 public:
    Trampoline(util::RequestSender<HullSpecificationProxy> reply, std::auto_ptr<PictureNamer> picNamer, Session& session)
        : m_reply(reply), m_picNamer(picNamer), m_session(session)
        {
            if (m_picNamer.get() == 0) {
                m_picNamer.reset(new game::spec::info::NullPictureNamer());
            }
        }

    void setExistingShipId(Id_t id);
    void setQuery(ShipQuery q);

    void describeWeaponEffects(game::spec::info::WeaponEffects& result);
    void describeHullFunctionDetails(game::spec::info::AbilityDetails_t& result, bool useNormalPictures);

    void sendResponse(const ShipList& shipList, const Root& root, const Turn* pTurn, const Game& game);
    void packResponse(HullSpecification& result, const ShipList& shipList, const Root& root, const Turn* pTurn, const Game& game);

 private:
    util::RequestSender<HullSpecificationProxy> m_reply;
    std::auto_ptr<PictureNamer> m_picNamer;
    Session& m_session;
    ShipQuery m_query;
};


void
game::proxy::HullSpecificationProxy::Trampoline::setExistingShipId(Id_t id)
{
    const Game* pGame         = m_session.getGame().get();
    const ShipList* pShipList = m_session.getShipList().get();
    const Root* pRoot         = m_session.getRoot().get();
    if (pGame != 0 && pShipList != 0 && pRoot != 0) {
        const Turn* pTurn = pGame->getViewpointTurn().get();
        if (pTurn != 0) {
            m_query.initForExistingShip(pTurn->universe(), id, *pShipList, pRoot->hostConfiguration(), pGame->shipScores());
            sendResponse(*pShipList, *pRoot, pTurn, *pGame);
        }
    }
}

void
game::proxy::HullSpecificationProxy::Trampoline::setQuery(ShipQuery q)
{
    const Game* pGame         = m_session.getGame().get();
    const ShipList* pShipList = m_session.getShipList().get();
    const Root* pRoot         = m_session.getRoot().get();
    if (pGame != 0 && pShipList != 0 && pRoot != 0) {
        const Turn* pTurn = pGame->getViewpointTurn().get();
        if (pTurn != 0) {
            m_query = q;
            sendResponse(*pShipList, *pRoot, pTurn, *pGame);
        }
    }
}

void
game::proxy::HullSpecificationProxy::Trampoline::describeWeaponEffects(game::spec::info::WeaponEffects& result)
{
    const ShipList* pShipList = m_session.getShipList().get();
    const Root* pRoot         = m_session.getRoot().get();
    if (pShipList != 0 && pRoot != 0) {
        game::spec::info::describeWeaponEffects(result, m_query, *pShipList, *pRoot, m_session.translator());
    }
}

void
game::proxy::HullSpecificationProxy::Trampoline::describeHullFunctionDetails(game::spec::info::AbilityDetails_t& result, bool useNormalPictures)
{
    const Game* pGame         = m_session.getGame().get();
    const ShipList* pShipList = m_session.getShipList().get();
    const Root* pRoot         = m_session.getRoot().get();
    const Turn* pTurn         = (pGame != 0 ? pGame->getViewpointTurn().get() : 0);
    if (pGame != 0 && pShipList != 0 && pRoot != 0 && pTurn != 0) {
        // Obtain list
        game::spec::HullFunctionList hfList;
        m_query.enumerateShipFunctions(hfList, pTurn->universe(), *pShipList, pRoot->hostConfiguration(), true);
        hfList.simplify();
        hfList.sortForNewShip(m_query.getPlayerDisplaySet());

        // Format it
        game::spec::info::describeHullFunctionDetails(result, hfList, &m_query, *pShipList, *m_picNamer, useNormalPictures, *pRoot, m_session.translator());
    }
}

void
game::proxy::HullSpecificationProxy::Trampoline::sendResponse(const ShipList& shipList, const Root& root, const Turn* pTurn, const Game& game)
{
    HullSpecification result;
    packResponse(result, shipList, root, pTurn, game);
    m_reply.postRequest(&HullSpecificationProxy::sendUpdate, result);
}

void
game::proxy::HullSpecificationProxy::Trampoline::packResponse(HullSpecification& result, const ShipList& shipList, const Root& root, const Turn* pTurn, const Game& game)
{
    // ex shipspec.pas:ShowShipSpec (sort-of)
    if (const game::spec::Hull* pHull = shipList.hulls().get(m_query.getHullType())) {
        // Environment
        const HostConfiguration& config = root.hostConfiguration();
        const int player = game.getViewpointPlayer();

        // Hull type
        result.name = pHull->getName(shipList.componentNamer());
        result.image = m_picNamer->getHullPicture(*pHull);

        // Scalar parameters
        result.hullId       = pHull->getId();
        result.mass         = pHull->getMass();
        result.numEngines   = pHull->getNumEngines();
        result.techLevel    = pHull->getTechLevel();
        result.maxCrew      = pHull->getMaxCrew();
        result.maxCargo     = pHull->getMaxCargo();
        result.maxFuel      = pHull->getMaxFuel();
        result.maxBeams     = pHull->getMaxBeams();
        result.maxLaunchers = pHull->getMaxLaunchers();
        result.numBays      = pHull->getNumBays();

        // Mine hit damage
        result.mineHitDamage = pHull->getMineHitDamage(player, false, root.hostVersion(), config);

        // Fuel usages
        result.fuelBurnPerTurn  = pHull->getTurnFuelUsage(player, false, config);
        result.fuelBurnPerFight = pHull->getTurnFuelUsage(player, true,  config);

        // Cost
        result.cost = pHull->cost();

        // Build points
        if (root.hostVersion().isPBPGame(config)) {
            result.pointsToBuild      = pHull->getPointsToBuild     (player, root.hostVersion(), config);
            result.pointsForKilling   = pHull->getPointsForKilling  (player, root.hostVersion(), config);
            result.pointsForScrapping = pHull->getPointsForScrapping(player, root.hostVersion(), config);
            if (pTurn != 0) {
                result.pointsAvailable = game::score::CompoundScore(game.scores(), game::score::ScoreId_BuildPoints, 1).get(game.scores(), pTurn->getTurnNumber(), player).orElse(0);
            }
        }

        // Players
        result.players = shipList.hullAssignments().getPlayersForHull(root.hostConfiguration(), pHull->getId());

        // Abilities
        if (pTurn != 0) {
            game::spec::HullFunctionList hfList;
            m_query.enumerateShipFunctions(hfList, pTurn->universe(), shipList, root.hostConfiguration(), false);
            hfList.simplify();
            hfList.sortForNewShip(m_query.getPlayerDisplaySet());

            game::spec::info::describeHullFunctions(result.abilities, hfList, &m_query, shipList, *m_picNamer, root, m_session.translator());
        }
    }
}


/*
 *  TrampolineFromSession
 */

class game::proxy::HullSpecificationProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<HullSpecificationProxy> reply, std::auto_ptr<PictureNamer> picNamer)
        : m_reply(reply), m_picNamer(picNamer)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(m_reply, m_picNamer, session); }
 private:
    util::RequestSender<HullSpecificationProxy> m_reply;
    std::auto_ptr<PictureNamer> m_picNamer;
};


/*
 *  HullSpecificationProxy
 */

game::proxy::HullSpecificationProxy::HullSpecificationProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer)
    : m_reply(reply, *this),
      m_request(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender(), picNamer)))
{ }

void
game::proxy::HullSpecificationProxy::setExistingShipId(Id_t id)
{
    m_request.postRequest(&Trampoline::setExistingShipId, id);
}

void
game::proxy::HullSpecificationProxy::setQuery(const ShipQuery& q)
{
    m_request.postRequest(&Trampoline::setQuery, q);
}

void
game::proxy::HullSpecificationProxy::describeWeaponEffects(WaitIndicator& ind, game::spec::info::WeaponEffects& result)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(game::spec::info::WeaponEffects& result)
            : m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.describeWeaponEffects(m_result); }
     private:
        game::spec::info::WeaponEffects& m_result;
    };

    // Clear
    result = game::spec::info::WeaponEffects();

    // Retrieve result
    Task t(result);
    ind.call(m_request, t);
}

void
game::proxy::HullSpecificationProxy::describeHullFunctionDetails(WaitIndicator& ind, game::spec::info::AbilityDetails_t& result, bool useNormalPictures)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(game::spec::info::AbilityDetails_t& result, bool useNormalPictures)
            : m_result(result), m_useNormalPictures(useNormalPictures)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.describeHullFunctionDetails(m_result, m_useNormalPictures); }
     private:
        game::spec::info::AbilityDetails_t& m_result;
        bool m_useNormalPictures;
    };

    // Clear
    result.clear();

    // Retrieve result
    Task t(result, useNormalPictures);
    ind.call(m_request, t);
}

void
game::proxy::HullSpecificationProxy::sendUpdate(HullSpecification info)
{
    sig_update.raise(info);
}
