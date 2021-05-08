/**
  *  \file game/proxy/taskeditorproxy.hpp
  *  \brief Class game::proxy::TaskEditorProxy
  */
#ifndef C2NG_GAME_PROXY_TASKEDITORPROXY_HPP
#define C2NG_GAME_PROXY_TASKEDITORPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/data/stringlist.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "interpreter/process.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Task editor proxy.
        Bidirectional, asynchronous proxy for a interpreter::TaskEditor object. */
    class TaskEditorProxy {
     public:
        struct Status {
            afl::data::StringList_t commands;
            size_t pc;
            size_t cursor;
            bool isInSubroutineCall;
            bool valid;
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

        /** Set cursor.
            \param newCursor New cursor (0-based line number) */
        void setCursor(size_t newCursor);

        /** Signal: change of task text.
            Reported whenever the task changes, or a new task is selected. */
        afl::base::Signal<void(const Status&)> sig_change;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<TaskEditorProxy> m_reply;
        util::RequestSender<Trampoline> m_trampoline;
    };

} }

#endif
