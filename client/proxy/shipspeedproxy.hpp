/**
  *  \file client/proxy/shipspeedproxy.hpp
  *  \brief Class client::proxy::ShipSpeedProxy
  */
#ifndef C2NG_CLIENT_PROXY_SHIPSPEEDPROXY_HPP
#define C2NG_CLIENT_PROXY_SHIPSPEEDPROXY_HPP

#include "client/downlink.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "util/requestsender.hpp"

namespace client { namespace proxy {

    /** Bidirectional proxy for ship speed.

        Provides synchronous access for determining possible warp speed settings,
        and asynchronous access to change the warp speed.
        Changing warp speed will produce a regular object change callback (game::map::Object::sig_change)
        that can be received using ObjectObserver. */
    class ShipSpeedProxy {
     public:
        /** Status structure. */
        struct Status {
            /** Current speed of the selected ship/fleet. */
            int currentSpeed;

            /** Maximum speed.
                If this is 0, the constructor parameters were invalid (e.g. ship does not exist) and this proxy cannot be used. */
            int maxSpeed;

            /** Marker for hyperspeed (game::spec::HullFunction::Hyperdrive).
                If currentSpeed=hyperSpeedMarker, this ship is hyperjumping.
                Otherwise, currentSpeed is a warp factor. */
            int hyperSpeedMarker;
        };

        /** Constructor.
            \param gameSender Game sender
            \param shipId Id of ship or fleet whose speed to change */
        ShipSpeedProxy(util::RequestSender<game::Session> gameSender, game::Id_t shipId);

        /** Destructor. */
        ~ShipSpeedProxy();

        /** Get current status.
            \param link Downlink
            \return current parameters/status */
        Status getStatus(Downlink& link);

        /** Set speed.
            This will trigger a regular object change callback asynchronously.
            \param n New speed [0,getStatus().maxSpeed] */
        void setSpeed(int n);

     private:
        class Trampoline;
        util::SlaveRequestSender<game::Session,Trampoline> m_trampoline;
    };

} }

#endif
