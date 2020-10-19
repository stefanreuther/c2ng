/**
  *  \file game/proxy/lockproxy.cpp
  *  \brief Class game::proxy::LockProxy
  *
  *  FIXME: optimize for warp is still missing
  */

#include "game/proxy/lockproxy.hpp"
#include "game/game.hpp"
#include "game/map/locker.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using game::map::Point;

class game::proxy::LockProxy::Response : public util::Request<LockProxy> {
 public:
    Response(Point_t target, Flags_t flags, Point_t result)
        : m_target(target), m_flags(flags), m_result(result)
        { }
    virtual void handle(LockProxy& proxy)
        { proxy.postResult(m_target, m_flags, m_result); }
 private:
    Point_t m_target;
    Flags_t m_flags;
    Point_t m_result;
};

class game::proxy::LockProxy::Query : public util::Request<Session> {
 public:
    Query(Point_t target, Flags_t flags, const Limit& limit, util::RequestSender<LockProxy> reply)
        : m_target(target), m_flags(flags), m_limit(limit), m_reply(reply)
        { }
    virtual void handle(Session& session);
 private:
    void sendResponse(Point_t pt);

    Point_t m_target;
    Flags_t m_flags;
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
    Turn* pTurn = pGame ? pGame->getViewpointTurn().get() : 0;
    if (pRoot == 0 || pGame == 0 || pTurn == 0) {
        sendResponse(m_target);
        return;
    }
    const game::map::Universe& univ = pTurn->universe();

    // Determine mode
    const game::map::LockOptionDescriptor_t& mode = (m_flags.contains(Left) ? game::map::Lock_Left : game::map::Lock_Right);
    int32_t items = pRoot->userConfiguration()[mode]();

    game::map::Locker locker(m_target, univ.config());
    if (m_limit.active) {
        locker.setRangeLimit(m_limit.min, m_limit.max);
    }
    locker.setMarkedOnly(m_flags.contains(MarkedOnly));

    // if ((items & li_Planet) != 0 && (optimize_warp ^ !!getUserPreferences().ChartScannerWarpWells())) {
    //     // Optimize warp wells
    //     findPlanet(univ, locker);
    //     GPoint found_point = locker.getPoint();

    //     // Query current position
    //     bool hyperjumping;
    //     int current_pid;
    //     lockQueryLocation(hyperjumping, current_pid);

    //     // Can we optimize warp wells?
    //     int clicked_pid = univ.findPlanetAt(found_point);
    //     if (clicked_pid > 0
    //         && config.AllowGravityWells()
    //         && (!hyperjumping || !host.isPHost() || config.AllowHyperjumpGravWells())
    //         && clicked_pid != current_pid)
    //     {
    //         /* We try to find the edge of a gravity well unless
    //            - we're heading for deep space, i.e. no planet found
    //            - gravity wells are disabled
    //            - we're starting inside the same gravity well we started in,
    //            in this case we assume we want to move to the planet */
    //         const int wwrange = (host.isPHost() ? config.GravityWellRange() :
    //                              hyperjumping ? 2 : 3);

    //         // Start with the assumption that moving directly is the best choice.
    //         // Then try all points in warp well range.
    //         int32 best_dist   = lockQueryDistance(found_point);
    //         GPoint best_point = found_point;
    //         for (int dx = -wwrange; dx <= wwrange; ++dx) {
    //             for (int dy = -wwrange; dy <= wwrange; ++dy) {
    //                 GPoint new_point = GPoint(found_point.x + dx, found_point.y + dy);
    //                 int32 new_dist = lockQueryDistance(new_point);
    //                 if (new_dist >= 0
    //                     && (best_dist < 0 || new_dist < best_dist)
    //                     && univ.findGravityPlanetAt(new_point) == clicked_pid)
    //                 {
    //                     // Accept new point if it is valid, has a better metric than
    //                     // the previous one, and it is in the same warp well.
    //                     best_dist = new_dist;
    //                     best_point = new_point;
    //                 }
    //             }
    //         }

    //         // Move to found point, if any
    //         if (best_dist >= 0)
    //             userMoveScannerTo(best_point);
    //     } else {
    //         // No warp wells, so just move to found point
    //         userMoveScannerTo(found_point);
    //     }
    // } else
    {
        // Regular locking only
        locker.addUniverse(univ, items, 0);
        sendResponse(locker.getFoundPoint());
    }
}

void
game::proxy::LockProxy::Query::sendResponse(Point_t pt)
{
    m_reply.postNewRequest(new Response(m_target, m_flags, pt));
}

/*
 *  LockProxy
 */

game::proxy::LockProxy::LockProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : sig_result(),
      m_gameSender(gameSender),
      m_reply(reply, *this),
      m_limit(),
      m_lastTarget(),
      m_lastFlags(Flags_t::fromInteger(-1))     // guaranteed to compare inequal to valid flag values
{
    m_limit.active = false;
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
game::proxy::LockProxy::postQuery(Point_t target, Flags_t flags)
{
    m_lastTarget = target;
    m_lastFlags = flags;
    m_gameSender.postNewRequest(new Query(target, flags, m_limit, m_reply.getSender()));
}

void
game::proxy::LockProxy::postResult(Point_t target, Flags_t flags, Point_t result)
{
    if (m_lastTarget == target && m_lastFlags == flags) {
        sig_result.raise(result);
    }
}
