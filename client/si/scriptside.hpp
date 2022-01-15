/**
  *  \file client/si/scriptside.hpp
  *  \brief Class client::si::ScriptSide
  */
#ifndef C2NG_CLIENT_SI_SCRIPTSIDE_HPP
#define C2NG_CLIENT_SI_SCRIPTSIDE_HPP

#include <set>
#include <vector>
#include "afl/base/signalconnection.hpp"
#include "afl/base/weaktarget.hpp"
#include "afl/data/value.hpp"
#include "client/si/scripttask.hpp"
#include "game/extra.hpp"
#include "game/session.hpp"
#include "interpreter/process.hpp"
#include "util/request.hpp"
#include "util/requestsender.hpp"

namespace client { namespace si {

    class ContextProvider;
    class Control;
    class RequestLink1;
    class RequestLink2;
    class ScriptProcedure;
    class UserSide;
    class UserTask;

    /** Script/UI Interaction: Script Side.

        This object is accessible as a Session extra.
        It can receive requests from UserSide, and send requests to it.
        See UserSide and Control for main documentation.

        Essentially, ScriptSide is to UserSide what a Trampoline is to most proxy classes.
        However, ScriptSide needs to be accessible from scripts.

        One major pattern is
        - call executeTaskWait or continueProcessWait to start a script process and associate a waitId with it
        - receive onTaskComplete() callback with the waitId */
    class ScriptSide : public game::Extra, public afl::base::WeakTarget {
     public:
        /** Constructor.
            @param reply RequestSender to send requests back to UserSide */
        ScriptSide(util::RequestSender<UserSide> reply, game::Session& session);

        /** Destructor. */
        ~ScriptSide();

        /** Access the underlying RequestSender.
            You should normally use call() or postNewTask() to talk to the UserSide.
            This method is available as an escape mechanism if you cannot use these.
            Because this obviously lacks the integration with process statuses, it can be only used for quick fire-and-forget tasks.
            @return Sender */
        util::RequestSender<UserSide> sender();

        /** Access underlying session.
            @return session */
        game::Session& session()
            { return m_session; }


        /*
         *  Starting new processes
         *
         *  These functions execute the given request, and call back onTaskComplete() with the wait Id.
         *  onTaskComplete() will reflect the result to the UserSide.
         *  These functions are invoked via UserSide.
         */

        /** Execute a script-based task.
            The task will be given a new process group, and can populate that with processes.
            Those will be run; completion of the process group will be signalled with onTaskComplete() for the given waitId.

            @param waitId   Wait Id for the onTaskComplete() callback
            @param task     The task; must not be null */
        void executeTaskWait(uint32_t waitId, std::auto_ptr<ScriptTask> task);

        /** Continue a detached process.
            Executes the process identified by the given RequestLink2 (and all other processes in the same process group).
            After execution finishes (and possibly generates callbacks to the UserSide),
            it will eventually call onTaskComplete() which will reflect the result to the UserSide.

            This function is intended to resume a detached process (detachProcess()) with a new wait Id.
            @param waitId   Wait Id
            @param link     Process identification */
        void continueProcessWait(uint32_t waitId, RequestLink2 link);


        /*
         *  Request Submission
         *
         *  These functions are invoked from script code on the script side,
         *  and submit tasks to the user side.
         */

        /** Post a task to the UserSide.
            This will suspend the specified process.
            The UserTask will be executed as an interaction on the user-interface side.
            It must eventually call continueProcess() or continueProcessWithFailure(),
            by using the corresponding functions of UserSide (which will call back into ScriptSide).

            This does not report the task or its wait finished.

            @param link  Initiating process
            @param t     Task (not null; newly-allocated; ownership transferred to ScriptSide)

            @see postNewInteraction */
        void postNewTask(RequestLink1 link, UserTask* t);

        /** Post an interaction request to the UserSide.
            An interaction is allowed to do user-interface interactions.

            @param req   Task */
        void postNewInteraction(util::Request<UserSide>* req);

        /** Execute command on UserSide.
            Synchronously executes the given Request on the user-interface side.
            This means you can pass parameters into the call and results out of the call using the Request object.

            The task is not allowed to block.
            It can, for example, retrieve widget content, retrieve font metrics, update a widget, etc.

            To execute a blocking task (e.g. a dialog), use postNewTask() and have the task continue your process,
            or use postNewInteraction().

            @param t Command
            @throw interpreter::Error if t.handle() throws */
        void call(util::Request<Control>& t);

        /** Execute command on UserSide, asynchronously.
            Executes the given Request without waiting for completion.
            This means you cannot pass parameters by reference, nor can you pass results back; exceptions are swallowed.

            This can be used as a higher-throughput version of call() in places where no results are needed.
            Note that requests are processed in sequence anyway, so even if this call is asynchronously,
            the Request will be guaranteed to have been processed before the next call().

            @param t Newly-allocated Request descendant. Will become owned by the ScriptSide. */
        void callAsyncNew(util::Request<Control>* t);


        /*
         *  Manipulating a running process
         *
         *  These functions are invoked via UserSide.
         */

        /** Continue process.
            This will execute the process and produce appropriate callbacks.
            The process will see a regular return (empty/no result) from the function that stopped it using postNewTask().

            @param link    Process identification

            @see interpreter::ProcessList::continueProcess  */
        void continueProcess(RequestLink2 link);

        /** Join processes into a process group.
            Moves process @c other into the same process group as @c link.
            Call continueProcess(link) next.

            @param link    Target process identification
            @param other   Other process identification

            @see interpreter::ProcessList::joinProcess  */
        void joinProcess(RequestLink2 link, RequestLink2 other);

        /** Join process group.
            Moves content of @c oldGroup into the same process group as @c link.
            Call continueProcess(link) next.

            @param link    Target process identification
            @param oldGroup Old process group

            @see interpreter::ProcessList::joinProcessGroup */
        void joinProcessGroup(RequestLink2 link, uint32_t oldGroup);

        /** Continue process with an error.
            This will execute the process and produce appropriate callbacks.
            The process will see an error return (exception) from the function that stopped it using postNewTask().

            @param link    Process identification
            @param error   Error message

            @see interpreter::ProcessList::continueProcessWithFailure */
        void continueProcessWithFailure(RequestLink2 link, String_t error);

        /** Detach process.
            This will (temporarily) release the process from our control,
            and satisfy the existing wait (onTaskComplete()).
            You must continue it later using continueProcessWait().

            @param link    Process identification */
        void detachProcess(RequestLink2 link);

        /** Set variable in a process.
            @param link    Process identification
            @param name    Variable name
            @param value   Value

            @see interpreter::Process::setVariable */
        void setVariable(RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value);


        /*
         *  Running Processes
         */

        /** Run processes.
            Executes all pending processes.

            For now, this function is exported to run processes that are not managed by ScriptSide/UserSide. */
        void runProcesses();


     private:
        /** Containing session. */
        game::Session& m_session;

        /** SignalConnection for interpreter::ProcessList::sig_processGroupFinish */
        afl::base::SignalConnection conn_processGroupFinish;

        /** SignalConnection for game::Session::sig_runRequest */
        afl::base::SignalConnection conn_runRequest;

        /** Sender to UserSide. */
        util::RequestSender<UserSide> m_reply;

        /** An active waitId/processGroupId association. */
        struct Wait {
            uint32_t waitId;
            uint32_t processGroupId;
            Wait(uint32_t waitId, uint32_t processGroupId)
                : waitId(waitId), processGroupId(processGroupId)
                { }
            Wait()
                : waitId(0), processGroupId(0)
                { }
        };
        std::vector<Wait> m_waits;

        /** Wait callback.
            Signals the wait result to the UserSide.
            @param id       Wait Id */
        void onTaskComplete(uint32_t waitId);

        /** Process group completion callback.
            Signals the appropriate waits.
            @param pgid    Completed process group */
        void onProcessGroupFinish(uint32_t pgid);

        /** Look up a wait for a process group.
            @param [in] pgid Process Group Id
            @param [out] out Wait
            @retval true  wait request found and removed
            @retval false no wait found */
        bool extractWait(uint32_t pgid, Wait& out);
    };

} }

#endif
