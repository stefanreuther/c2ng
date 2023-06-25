/**
  *  \file game/proxy/historyturnproxy.hpp
  *  \brief Class game::proxy::HistoryTurnProxy
  */
#ifndef C2NG_GAME_PROXY_HISTORYTURNPROXY_HPP
#define C2NG_GAME_PROXY_HISTORYTURNPROXY_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "game/historyturn.hpp"
#include "game/session.hpp"
#include "game/timestamp.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** History turn selection.
        Asynchronous, bidirectional proxy to retrieve and maintain the list of loaded history turns.

        To use,
        - call requestSetup() to retrieve initial list of turns (answers with sig_setup)
        - call requestUpdate() to resolve more unknown statuses (answers with sig_update), repeat as needed
        - call requestLoad() to load turn (answers with sig_update), repeat as needed

        As of 20230624, HistoryTurnProxy does not generate unsolicited callbacks (e.g. from a second instance working on the same game). */
    class HistoryTurnProxy {
     public:
        /** Status of a turn. */
        enum Status {
            Unknown,              ///< I don't know.
            Unavailable,          ///< I know it is not available.
            StronglyAvailable,    ///< I'm certain it's available.
            WeaklyAvailable,      ///< I guess it's available.
            Failed,               ///< Loading failed.
            Loaded,               ///< It is loaded.
            Current               ///< This is the current turn.
        };

        /** Information about a turn. */
        struct Item {
            int turnNumber;       ///< Turn number.
            Timestamp timestamp;  ///< Timestamp.
            Status status;        ///< Status.

            Item(int turnNumber, const Timestamp& timestamp, Status status)
                : turnNumber(turnNumber), timestamp(timestamp), status(status)
                { }
        };
        typedef std::vector<Item> Items_t;

        /** Constructor.
            @param sender Game sender
            @param reply  Receiver for responses */
        HistoryTurnProxy(util::RequestSender<Session> sender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~HistoryTurnProxy();

        /** Request initialisation.
            Asynchronously generates a list of turns and their status, without accessing I/O.
            Eventually answers with sig_setup.

            @param maxTurns Maximum number of turns to report.
                            Normally, this method returns all turns starting from 1.
                            If the game has more turns than this number, the oldest ones will not be accessible.
                            This is intended to avoid unbounded allocation
                            if the turn number is reported as an unlikely value such as 20000. */
        void requestSetup(int maxTurns);

        /** Request update of turn status.
            Accesses TurnLoader (and performs I/O) to turn "Unknown" turns into a better status.
            Automatically picks the turns to update.
            Eventually answers with sig_update.

            @param firstTurn First (oldest) turn we are interested in.
                             Normally, pass first turn given by sig_setup.
            @param maxTurns  Maximum number of turns to update.
                             Use to limit the amount of I/O performed before an answer is generated. */
        void requestUpdate(int firstTurn, int maxTurns);

        /** Request loading a turn.
            Accesses TurnLoader (and performs I/O) to try to turn a turn into "Loaded" (or "Failed") status.
            Eventually answers with sig_update.

            @param turnNumber Turn to load */
        void requestLoad(int turnNumber);

        /** Signal: Setup complete.
            @param items       List of all turns, in sequential order
            @param activeTurn  Active turn number */
        afl::base::Signal<void(const Items_t&, int)> sig_setup;

        /** Signal: Update turn list.
            @param items  Items to update. Use turnNumber as primary key to update UI-side list.
                          List is empty if there was a problem and you should not (automatically) request more. */
        afl::base::Signal<void(const Items_t&)> sig_update;

     private:
        class InitialResponse;
        class InitialRequest;
        class UpdateResponse;
        class UpdateRequest;
        class LoadRequest;

        util::RequestReceiver<HistoryTurnProxy> m_reply;
        util::RequestSender<Session> m_request;

        static void sendUpdateResponse(Session& session, util::RequestSender<HistoryTurnProxy>& response, Items_t& content);
    };

} }

#endif
