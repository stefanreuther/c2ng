/**
  *  \file client/proxy/lockproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_LOCKPROXY_HPP
#define C2NG_CLIENT_PROXY_LOCKPROXY_HPP

#include "afl/base/signal.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "game/map/point.hpp"
#include "util/requestreceiver.hpp"
#include "afl/bits/smallset.hpp"

namespace client { namespace proxy {

    class LockProxy {
     public:
        typedef game::map::Point Point_t;

        enum Flag {
            Left,
            MarkedOnly,
            OptimizeWarp
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        LockProxy(util::RequestDispatcher& reply, util::RequestSender<game::Session> gameSender);
        ~LockProxy();

        void setRangeLimit(Point_t min, Point_t max);
        void postQuery(Point_t target, Flags_t flags);

        afl::base::Signal<void(game::map::Point)> sig_result;

     private:
        class Response;
        class Query;

        struct Limit {
            bool active;
            Point_t min;
            Point_t max;
        };

        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<LockProxy> m_reply;

        Limit m_limit;

        Point_t m_lastTarget;
        Flags_t m_lastFlags;

        void postResult(Point_t target, Flags_t flags, Point_t result);
    };

} }

#endif
