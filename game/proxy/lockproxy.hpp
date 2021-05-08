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
        Provides a possibility to lock onto objects on the map (mouse click).

        Bidirectional, asynchronous:
        - lock (postQuery, sig_result)

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

        /** Post query.
            Schedules a sig_result callback with the result point.
            Note that the sig_result callback will be suppressed if postQuery() is called for a different query
            before the previous one has been answered (de-bouncing).
            \param target Clicked position
            \param flags  Flags */
        void postQuery(Point_t target, Flags_t flags);

        /** Signal: result.
            \param pt Position (unit nearest to \c target in last postQuery call) */
        afl::base::Signal<void(game::map::Point)> sig_result;

     private:
        class Response;
        class Query;

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

        void postResult(Point_t target, Flags_t flags, Point_t result);
    };

} }


#endif
