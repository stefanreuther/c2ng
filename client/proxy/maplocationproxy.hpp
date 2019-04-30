/**
  *  \file client/proxy/maplocationproxy.hpp
  *  \brief Class client::proxy::MapLocationProxy
  */
#ifndef C2NG_CLIENT_PROXY_MAPLOCATIONPROXY_HPP
#define C2NG_CLIENT_PROXY_MAPLOCATIONPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "game/map/configuration.hpp"
#include "util/slaverequestsender.hpp"

namespace client { namespace proxy {

    /** Asynchronous, bidirectional proxy for map location.
        This accesses the game::Session > game::Game > game::map::Cursors > game::map::Location object. */
    class MapLocationProxy {
     public:
        /** Constructor.
            \param reply RequestDispatcher to receive replies.
            \param gameSender RequestSender to send to the game */
        MapLocationProxy(util::RequestDispatcher& reply, util::RequestSender<game::Session> gameSender);

        /** Destructor. */
        ~MapLocationProxy();

        /** Post a request to query the current location.
            Every postQueryLocation() call will eventually produce a sig_locationResult callback. */
        void postQueryLocation();

        /** Set location to point.
            \param pt Point
            \see game::map::Location::set() */
        void setPosition(game::map::Point pt);

        /** Set location to reference.
            \param ref Reference
            \see game::map::Location::set() */
        void setPosition(game::Reference ref);

        /** Location callback.
            Called in response to postQueryLocation().
            \param ref Reference result
            \param pt Point result */
        afl::base::Signal<void(game::Reference, game::map::Point, game::map::Configuration)> sig_locationResult;

        /** Position change callback.
            Called if the game-side location reports a sig_positionChange. */
        afl::base::Signal<void(game::map::Point)> sig_positionChange;

     private:
        class Trampoline;
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<MapLocationProxy> m_reply;
        util::SlaveRequestSender<game::Session, Trampoline> m_trampoline;
    };

} }

#endif
