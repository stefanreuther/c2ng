/**
  *  \file game/proxy/taskeditorproxy.hpp
  *  \brief Class game::proxy::TaskEditorProxy
  */
#ifndef C2NG_GAME_PROXY_TASKEDITORPROXY_HPP
#define C2NG_GAME_PROXY_TASKEDITORPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/data/stringlist.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "interpreter/process.hpp"
#include "util/numberformatter.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Task editor proxy.
        Bidirectional, asynchronous proxy for a interpreter::TaskEditor object and some related objects:
        - game::interface::ShipPredictor
        - game::interface::NotificationStore
        - game::actions::BuildShip */
    class TaskEditorProxy {
     public:
        /** Task status. */
        struct Status {
            afl::data::StringList_t commands;        ///< List of commands.
            size_t pc;                               ///< Program counter.
            size_t cursor;                           ///< Cursor.
            bool isInSubroutineCall;                 ///< true if call is in subroutine call, false if at start of instruction.
            bool valid;                              ///< Validity flag.
            Status()
                : commands(), pc(), cursor(), isInSubroutineCall(), valid()
                { }
        };

        /** Ship status. */
        struct ShipStatus {
            game::map::Point startPosition;          ///< Starting position.
            std::vector<game::map::Point> positions; ///< Future positions; see game::interface::ShipTaskPredictor::getPosition().
            std::vector<long> distances2;            ///< Distances to future positions
            size_t numFuelPositions;                 ///< Number of positions for which there is enough fuel; see game::interface::ShipTaskPredictor::getNumFuelPositions().
            int currentTurn;                         ///< Current turn number.
            int numTurns;                            ///< Number of turns computed; see game::interface::ShipTaskPredictor::getNumTurns().
            int numFuelTurns;                        ///< Number of turns for which there is fuel; see game::interface::ShipTaskPredictor::getNumFuelTurns().
            int startingFuel;                        ///< Starting fuel amount.
            int movementFuel;                        ///< Movement fuel used; see game::interface::ShipTaskPredictor::getMovementFuel().
            int cloakFuel;                           ///< Cloak fuel used; see game::interface::ShipTaskPredictor::getCloakFuel().
            int remainingFuel;                       ///< Remaining fuel; see game::interface::ShipTaskPredictor::getRemainingFuel().
            util::NumberFormatter numberFormatter;   ///< NumberFormatter to use for formatting fuel amounts.
            bool isHyperdriving;                     ///< true if ship is hyperwarping at end; see game::interface::ShipTaskPredictor::isHyperdriving().
            bool valid;                              ///< Validity flag.
            ShipStatus()
                : startPosition(), positions(), distances2(), numFuelPositions(), currentTurn(), numTurns(), numFuelTurns(),
                  startingFuel(), movementFuel(), cloakFuel(), remainingFuel(), numberFormatter(false, false), isHyperdriving(), valid()
                { }
        };

        /** Starbase status. */
        struct BaseStatus {
            afl::data::StringList_t buildOrder;      ///< Build order in textual form. Empty if there is no build order. See ShipBuildOrder::describe().
            String_t missingMinerals;                ///< Missing minerals in textual form. Empty if there is nothing missing.
            BaseStatus()
                : buildOrder(), missingMinerals()
                { }
        };

        /** Notification message status. */
        struct MessageStatus {
            bool hasUnconfirmedMessage;              ///< true if unconfirmed message exists (validity flag).
            String_t text;                           ///< Text of unconfirmed message.
            MessageStatus()
                : hasUnconfirmedMessage(), text()
                { }
        };

        /** Constructor.
            \param gameSender Sender
            \param reply RequestDispatcher to receive replies back */
        TaskEditorProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~TaskEditorProxy();

        /** Select task to show in this TaskEditorProxy.
            Function will respond with a sig_change.
            \param id Object Id
            \param kind Task kind
            \param create true to create task if necessary
            \see game::Session::getAutoTaskEditor */
        void selectTask(Id_t id, interpreter::Process::ProcessKind kind, bool create);

        /** Get status, synchronously.
            \param [in,out] ind  WaitIndicator
            \param [out]    out  Status */
        void getStatus(WaitIndicator& ind, Status& out);

        /** Set cursor.
            \param newCursor New cursor (0-based line number) */
        void setCursor(size_t newCursor);

        /** Add command as current command.
            \param cmd Command
            \see interpreter::TaskEditor::addAsCurrent() */
        void addAsCurrent(const String_t& cmd);

        /** Add command at end of task.
            \param cmd Command
            \see interpreter::TaskEditor::addAtEnd() */
        void addAtEnd(const String_t& cmd);

        /** Signal: change of task text.
            Reported whenever the task changes, or a new task is selected. */
        afl::base::Signal<void(const Status&)> sig_change;

        /** Signal: change of ship prediction. */
        afl::base::Signal<void(const ShipStatus&)> sig_shipChange;

        /** Signal: change of starbase prediction.
            Reports the build order the cursor is on. */
        afl::base::Signal<void(const BaseStatus&)> sig_baseChange;

        /** Signal: change of notification message status. */
        afl::base::Signal<void(const MessageStatus&)> sig_messageChange;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<TaskEditorProxy> m_reply;
        util::RequestSender<Trampoline> m_trampoline;
    };

} }

#endif
