/**
  *  \file game/proxy/lockproxy.cpp
  *  \brief Class game::proxy::LockProxy
  */

#include "game/proxy/lockproxy.hpp"
#include "game/game.hpp"
#include "game/map/locker.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using game::map::Point;
using game::config::UserConfiguration;

class game::proxy::LockProxy::Query : public util::Request<Session> {
 public:
    Query(Point_t target, Flags_t flags, const Limit& limit, const afl::base::Optional<util::Atom_t>& tagFilter, const Origin& origin, util::RequestSender<LockProxy> reply)
        : m_target(target), m_flags(flags), m_limit(limit), m_tagFilter(tagFilter), m_origin(origin), m_reply(reply)
        { }
    virtual void handle(Session& session);
 private:
    void sendResponse(Point_t pt);

    Point_t m_target;
    Flags_t m_flags;
    Limit m_limit;
    afl::base::Optional<util::Atom_t> m_tagFilter;
    Origin m_origin;
    util::RequestSender<LockProxy> m_reply;
};

class game::proxy::LockProxy::UnitNameQuery : public util::Request<Session> {
 public:
    UnitNameQuery(Point_t target, const Limit& limit, util::RequestSender<LockProxy> reply)
        : m_target(target), m_limit(limit), m_reply(reply)
        { }
    virtual void handle(Session& session);
 private:
    void sendResponse(Point_t pt, String_t name);

    Point_t m_target;
    Limit m_limit;
    util::RequestSender<LockProxy> m_reply;
};

/*
 *  Query
 */

void
game::proxy::LockProxy::Query::handle(Session& session)
{
    // ex WScannerChartWidget::doItemLock
    // We need a root, a game, and a viewpoint turn. If we haven't, just respond with a dummy answer.
    Root* pRoot = session.getRoot().get();
    Game* pGame = session.getGame().get();
    Turn* pTurn = pGame ? &pGame->viewpointTurn() : 0;
    if (pRoot == 0 || pGame == 0 || pTurn == 0) {
        sendResponse(m_target);
        return;
    }
    const game::map::Universe& univ = pTurn->universe();

    // Determine mode
    const game::map::LockOptionDescriptor_t& mode = (m_flags.contains(Left) ? UserConfiguration::Lock_Left : UserConfiguration::Lock_Right);
    int32_t items = pRoot->userConfiguration()[mode]();
    if (m_flags.contains(NoDrawings)) {
        items &= ~game::map::MatchDrawings;
    }

    game::map::Locker locker(m_target, pGame->mapConfiguration());
    if (m_limit.active) {
        locker.setRangeLimit(m_limit.min, m_limit.max);
    }
    locker.setDrawingTagFilter(m_tagFilter);
    locker.setMarkedOnly(m_flags.contains(MarkedOnly));

    // Find target
    locker.addUniverse(univ, items, 0);

    // Optimize warp.
    // @diff PCC2 only locks at planets when it detects this.
    bool actionWarp = m_flags.contains(ToggleOptimizeWarp);
    bool configWarp = pRoot->userConfiguration()[UserConfiguration::ChartScannerWarpWells]();

    game::spec::ShipList* pShipList = session.getShipList().get();
    if (m_origin.active && (items & game::map::MatchPlanets) != 0 && (actionWarp != configWarp) && pShipList != 0) {
        // Warp-well aware
        sendResponse(locker.findWarpWellEdge(m_origin.pos,
                                             m_origin.isHyperdriving,
                                             univ,
                                             m_origin.shipId,
                                             pGame->shipScores(),
                                             *pShipList,
                                             pRoot->hostConfiguration(),
                                             pRoot->hostVersion(),
                                             pRoot->registrationKey()));
    } else {
        // Regular locking only
        sendResponse(locker.getFoundPoint());
    }
}

void
game::proxy::LockProxy::Query::sendResponse(Point_t pt)
{
    m_reply.postRequest(&LockProxy::postResult, m_target, m_flags, pt);
}

/*
 *  UnitNameQuery
 */

void
game::proxy::LockProxy::UnitNameQuery::handle(Session& session)
{
    // ex WScannerChartWidget::doTooltip (part)
    Root* pRoot = session.getRoot().get();
    Game* pGame = session.getGame().get();
    Turn* pTurn = pGame ? &pGame->viewpointTurn() : 0;
    if (pRoot == 0 || pGame == 0 || pTurn == 0) {
        sendResponse(m_target, String_t());
        return;
    }
    const game::map::Universe& univ = pTurn->universe();

    // Determine mode
    game::map::Locker locker(m_target, pGame->mapConfiguration());
    if (m_limit.active) {
        locker.setRangeLimit(m_limit.min, m_limit.max);
    }

    // Find target
    locker.addUniverse(univ, game::map::MatchPlanets | game::map::MatchShips, 0);

    sendResponse(locker.getFoundPoint(),
                 univ.findLocationUnitNames(locker.getFoundPoint(), pGame->getViewpointPlayer(), pRoot->playerList(), pGame->mapConfiguration(), session.translator(), session.interface()));
}

void
game::proxy::LockProxy::UnitNameQuery::sendResponse(Point_t pt, String_t name)
{
    m_reply.postRequest(&LockProxy::postUnitNameResult, m_target, pt, name);
}

/*
 *  LockProxy
 */

game::proxy::LockProxy::LockProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : sig_result(),
      m_gameSender(gameSender),
      m_reply(reply, *this),
      m_limit(),
      m_origin(),
      m_tagFilter(),
      m_lastTarget(),
      m_lastFlags(Flags_t::fromInteger(-1))     // guaranteed to compare inequal to valid flag values
{
    m_limit.active = false;
    m_origin.active = false;
}

game::proxy::LockProxy::~LockProxy()
{ }

void
game::proxy::LockProxy::setRangeLimit(Point_t min, Point_t max)
{
    m_limit.active = true;
    m_limit.min = min;
    m_limit.max = max;
}

void
game::proxy::LockProxy::setDrawingTagFilter(afl::base::Optional<util::Atom_t> tagFilter)
{
    m_tagFilter = tagFilter;
}

void
game::proxy::LockProxy::setOrigin(Point_t pos, bool isHyperdriving, Id_t shipId)
{
    m_origin.active = true;
    m_origin.isHyperdriving = isHyperdriving;
    m_origin.pos = pos;
    m_origin.shipId = shipId;
}

void
game::proxy::LockProxy::requestPosition(Point_t target, Flags_t flags)
{
    m_lastTarget = target;
    m_lastFlags = flags;
    m_gameSender.postNewRequest(new Query(target, flags, m_limit, m_tagFilter, m_origin, m_reply.getSender()));
}

void
game::proxy::LockProxy::requestUnitNames(Point_t target)
{
    m_lastUnitNameTarget = target;
    m_gameSender.postNewRequest(new UnitNameQuery(target, m_limit, m_reply.getSender()));
}

void
game::proxy::LockProxy::postResult(Point_t target, Flags_t flags, Point_t result)
{
    if (m_lastTarget == target && m_lastFlags == flags) {
        sig_result.raise(result);
    }
}

void
game::proxy::LockProxy::postUnitNameResult(Point_t target, Point_t result, String_t name)
{
    if (m_lastUnitNameTarget == target) {
        sig_unitNameResult.raise(result, name);
    }
}
