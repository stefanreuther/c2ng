/**
  *  \file client/proxy/taskeditorproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_TASKEDITORPROXY_HPP
#define C2NG_CLIENT_PROXY_TASKEDITORPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/data/stringlist.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "interpreter/process.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"

namespace client { namespace proxy {

    class TaskEditorProxy {
     public:
        struct Status {
            afl::data::StringList_t commands;
            size_t pc;
            size_t cursor;
            bool isInSubroutineCall;
            bool valid;
        };

        TaskEditorProxy(util::RequestDispatcher& reply, util::RequestSender<game::Session> gameSender);

        ~TaskEditorProxy();

        void selectTask(game::Id_t id, interpreter::Process::ProcessKind kind, bool create);

        void setCursor(size_t newCursor);


        afl::base::Signal<void(const Status&)> sig_change;

     private:
        class Trampoline;
        util::RequestReceiver<TaskEditorProxy> m_reply;
        util::SlaveRequestSender<game::Session, Trampoline> m_trampoline;
    };

} }

#endif
