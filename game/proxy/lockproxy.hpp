/**
  *  \file game/proxy/lockproxy.hpp
  *  \brief Class game::proxy::LockProxy
  */
#ifndef C2NG_GAME_PROXY_LOCKPROXY_HPP
#define C2NG_GAME_PROXY_LOCKPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/bits/smallset.hpp"
#include "game/map/point.hpp"
#include "game/session.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Starchart lock proxy.
        Provides a possibility to lock onto objects on the map (mouse click) and retrieve names (mouse hover).

        Bidirectional, asynchronous:
        - lock (requestPosition, sig_result)
        - retrieve names (requestUnitNames, sig_unitNameResult)

        \see game::map::Locker */
    class LockProxy {
     public:
        /** Shortcut: position. */
        typedef game::map::Point Point_t;

        /** Option flag. */
        enum Flag {
            Left,                     /**< Use Lock_Left (default: Lock_Right). */
            MarkedOnly,               /**< Consider only marked objects. \see game::map::Locker::setMarkedOnly */
            ToggleOptimizeWarp        /**< Optimize for movement different from global config. */
        };

        /** Option flags. */
        typedef afl::bits::SmallSet<Flag> Flags_t;


        /** Constructor.
            \param gameSender Game sender
            \param reply RequestDispatcher to receive replies */
        LockProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~LockProxy();

        /** Set range limit for future queries.
            \param min Minimum (bottom-left) coordinate, inclusive.
            \param max Maximum (top-right) coordinate, inclusive.
            \see game::map::Locker::setRangeLimit */
        void setRangeLimit(Point_t min, Point_t max);

        /** Set origin for movement-aware locking to warp-well edges.
            \param pos Position
            \param isHyperdriving true if hyperdriving */
        void setOrigin(Point_t pos, bool isHyperdriving);

        /** Request position lock.
            Determines the object closest to the clicked target and schedules a sig_result callback with the result point.
            Note that the sig_result callback will be suppressed if requestPosition() is called for a different query
            before the previous one has been answered (de-bouncing).
            \param target Clicked position
            \param flags  Flags
            \see game::map::Locker::addUniverse, game::map::Locker::getFoundPoint */
        void requestPosition(Point_t target, Flags_t flags);

        /** Request unit names for a location.
            Determines the point closest to the target and schedules a sig_nameResult callback with the result point and name.
            Note that the sig_result callback will be suppressed if requestName() is called for a different query
            before the previous one has been answered (de-bouncing).
            \param target Clicked position
            \see game::map::Universe::findLocationUnitNames */
        void requestUnitNames(Point_t target);

        /** Signal: position result.
            \param pt Position (unit nearest to \c target in last requestPosition call) */
        afl::base::Signal<void(game::map::Point)> sig_result;

        /** Signal: unit name result.
            \param pt Position (unit nearest to \c target in last requestUnitNames call)
            \param name Name result (multi-line string) */
        afl::base::Signal<void(game::map::Point, String_t)> sig_unitNameResult;

     private:
        class Query;
        class UnitNameQuery;

        struct Limit {
            bool active;
            Point_t min;
            Point_t max;
        };

        struct Origin {
            bool active;
            bool isHyperdriving;
            Point_t pos;
        };

        util::RequestSender<Session> m_gameSender;
        util::RequestReceiver<LockProxy> m_reply;

        Limit m_limit;
        Origin m_origin;

        Point_t m_lastTarget;
        Flags_t m_lastFlags;

        Point_t m_lastUnitNameTarget;

        void postResult(Point_t target, Flags_t flags, Point_t result);
        void postUnitNameResult(Point_t target, Point_t result, String_t name);
    };

} }

#endif
