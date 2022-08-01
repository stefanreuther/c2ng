/**
  *  \file game/proxy/historyshipproxy.hpp
  *  \brief Class game::proxy::HistoryShipProxy
  */
#ifndef C2NG_GAME_PROXY_HISTORYSHIPPROXY_HPP
#define C2NG_GAME_PROXY_HISTORYSHIPPROXY_HPP

#include <memory>
#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "game/map/shipinfo.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** History ship proxy.
        Provides access to game::map::HistoryShipType / game::map::Cursors::currentHistoryShip().

        Bidirectional, asynchronous:
        - browse through ships (browseAt)
        - receive updates for history information (sig_change)

        To retrieve further values, use a CursorObserverProxy or similar.
        To use other browser modes, modify currentHistoryShip() directly, e.g. from a script.

        TODO: should this implement ObjectObserver?
        TODO: should this implement browse(ObjectCursor::Mode)?
        TODO: how to initialize cursor when coming from a HistoryShip dialog looking at a position? */
    class HistoryShipProxy {
     public:
        /** Status report. */
        struct Status {
            /** Currently-selected ship. */
            Id_t shipId;

            /** Ship locations, starting with most recent turn.
                @see game::map::packShipLocationInfo() */
            game::map::ShipLocationInfos_t locations;

            /** Turn number hint.
                Set when this report is the result of a browsing operation,
                to place the cursor on an appropriate position.
                Unset when the report is the result of an unrelated change on the ship. */
            afl::base::Optional<int> turnNumber;

            Status()
                : shipId(), locations(), turnNumber()
                { }
        };

        /** Browse mode. */
        enum Mode {
            Next,               ///< Find next ship.
            Previous,           ///< Find previous ship.
            First,              ///< Find lowest-Id ship.
            Last                ///< Find highest-Id ship.
        };

        /** Constructor.
            After creation, a report about the current situation will immediately be generated.
            @param gameSender Game sender
            @param reply      RequestDispatcher to receive updates in this thread */
        HistoryShipProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~HistoryShipProxy();

        /** Browse through ships at a position.
            If a different ship can be found that is/was at the specified position,
            places cursor on it and sends an update (sig_change).
            @param pt      Position
            @param mode    Browse mode
            @param marked  true to accept only marked ships */
        void browseAt(game::map::Point pt, Mode mode, bool marked);

        /** Signal: ship/location update. */
        afl::base::Signal<void(const Status&)> sig_change;

     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<HistoryShipProxy> m_reply;
        util::RequestSender<Trampoline> m_request;

        void sendUpdate(std::auto_ptr<Status> st);
    };

} }

#endif
