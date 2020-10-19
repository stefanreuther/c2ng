/**
  *  \file game/proxy/processlistproxy.hpp
  *  \brief Class game::proxy::ProcessListProxy
  */
#ifndef C2NG_GAME_PROXY_PROCESSLISTPROXY_HPP
#define C2NG_GAME_PROXY_PROCESSLISTPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/interface/processlisteditor.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"

namespace game { namespace proxy {

    /** Bidirectional proxy for process list access.
        This proxies a game::interface::ProcessListEditor object operating on a game::Session.

        Bidirectional synchronous: initialisation
        - call init() to retrieve initial list of processes
        - call commit() to finalize

        Bidirectional asynchronous: modification
        - call setProcessState, setAllProcessState, setProcessPriority to invoke the respective ProcessListEditor mathods
        - retrieve updates on sig_listChange */
    class ProcessListProxy {
     public:
        /** Target state of process (shortcut to ProcessListEditor). */
        typedef game::interface::ProcessListEditor::State State_t;

        /** Human-readable process information (shortcut to ProcessListEditor). */
        typedef game::interface::ProcessListEditor::Info Info_t;

        /** List of process information. */
        typedef std::vector<Info_t> Infos_t;


        /** Constructor.
            \param gameSender Game sender
            \param reply      RequestDispatcher to receive updates */
        ProcessListProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~ProcessListProxy();

        /** Initialize and retrieve initial process list.
            \param link         WaitIndicator object for UI synchronisation
            \param result [out] Result */
        void init(WaitIndicator& link, Infos_t& result);

        /** Prepare a state change.
            The change will be executed when commit() is called.
            The new prepared state will be reported on sig_listChange.
            \param pid   Process id
            \param state Target state */
        void setProcessState(uint32_t pid, State_t state);

        /** Prepare a state change for all processes.
            The changes will be executed when commit() is called.
            The new prepared state will be reported on sig_listChange.
            \param state Target state */
        void setAllProcessState(State_t state);

        /** Set process priority.
            The updated priority will be reported on sig_listChange.
            \param pid   Process id
            \param pri   Priority */
        void setProcessPriority(uint32_t pid, int pri);

        /** Perform all prepared state changes.
            This will create a process group and place processes in it.
            Caller needs to run that process group.
            \param link WaitIndicator object for UI synchronisation
            \return process group Id */
        uint32_t commit(WaitIndicator& link);

        /** Signal: updated process list. */
        afl::base::Signal<void(const Infos_t&)> sig_listChange;

     private:
        class Trampoline;

        util::RequestReceiver<ProcessListProxy> m_reply;
        util::SlaveRequestSender<Session, Trampoline> m_request;
    };

} }

#endif
