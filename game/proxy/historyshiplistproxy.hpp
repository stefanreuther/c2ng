/**
  *  \file game/proxy/historyshiplistproxy.hpp
  *  \brief Class game::proxy::HistoryShipListProxy
  */
#ifndef C2NG_GAME_PROXY_HISTORYSHIPLISTPROXY_HPP
#define C2NG_GAME_PROXY_HISTORYSHIPLISTPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/ref/historyshiplist.hpp"
#include "game/ref/historyshipselection.hpp"
#include "game/session.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** History ship list proxy.

        Maintains a game::ref::HistoryShipList, defined by a game::ref::HistoryShipSelection,
        and reports updates.

        Bidirectional, asynchronous:
        - define the selection
        - receive list updates on sig_listChange */
    class HistoryShipListProxy {
     public:
        /** Constructor.
            \param gameSender Game sender
            \param reply RequestDispatcher to receive replies in this thread */
        HistoryShipListProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~HistoryShipListProxy();

        /** Set current selection.
            Will always respond with a sig_listChange.
            Will send additional updates when the universe changes.

            setSelection always applies to the viewpoint turn at the time of the call.
            It will not follow a later turn change
            (although the reported ship names are always taken from the current viewpoint turn).

            \param sel Selection */
        void setSelection(const game::ref::HistoryShipSelection& sel);

        /** Signal: new content.
            \param list Content */
        afl::base::Signal<void(const game::ref::HistoryShipList&)> sig_listChange;

     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<HistoryShipListProxy> m_reply;
        util::RequestSender<Trampoline> m_request;

        void updateList(game::ref::HistoryShipList list);
    };

} }

#endif
