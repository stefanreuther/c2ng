/**
  *  \file client/si/scriptside.hpp
  */
#ifndef C2NG_CLIENT_SI_SCRIPTSIDE_HPP
#define C2NG_CLIENT_SI_SCRIPTSIDE_HPP

#include <vector>
#include <set>
#include "afl/base/signalconnection.hpp"
#include "afl/data/value.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"
#include "util/requestsender.hpp"
#include "util/slaveobject.hpp"
#include "client/si/requestlink2.hpp"
#include "afl/base/weaktarget.hpp"
#include "game/extra.hpp"

namespace client { namespace si {

    class ContextProvider;
    class RequestLink1;
    class RequestLink2;
    class ScriptProcedure;
    class UserSide;
    class UserTask;
    class UserCall;

    class ScriptSide : public game::Extra, public afl::base::WeakTarget {
     public:
        ScriptSide(util::RequestSender<UserSide> reply);
        ~ScriptSide();

        /*
         *  Starting new processes
         *
         *  These functions execute the given request, and call back handleWait() with the wait Id.
         *  handleWait() will reflect the result to the UserSide.
         */

        /** Execute a command.
            This will compile the command into a BCO, create a new process with that BCO, and run the process in a new process group.

            After execution finishes (and possibly generates callbacks to the UserSide),
            it will eventually call handleWait() which will reflect the result to the UserSide.
            \param id       Wait Id for the handleWait() callback
            \param session  Session to work on
            \param command  Command (string)
            \param verbose  false: just execute the command; true: log command and possible result on console
            \param name     Name of process
            \param ctxp     ContextProvider (null if none required). The ContextProvider provides initial context for the process */
        void executeCommandWait(uint32_t id, game::Session& session, String_t command, bool verbose, String_t name, std::auto_ptr<ContextProvider> ctxp);

        /** Execute a process group.
            After execution finishes (and possibly generates callbacks to the UserSide),
            it will eventually call handleWait() which will reflect the result to the UserSide.
            \param id       Wait Id
            \param pgid     Process group Id
            \param session  Session to work on */
        void executeProcessGroupWait(uint32_t id, uint32_t pgid, game::Session& session);

        /** Continue a detached process.
            Executes the process identified by the given RequestLink2 (and all other processes in the same process group).
            After execution finishes (and possibly generates callbacks to the UserSide),
            it will eventually call handleWait() which will reflect the result to the UserSide.

            This function is intended to resume a detached process (detachProcess()) with a new wait Id.
            \param id       Wait Id
            \param session  Session to work on
            \param link     Process identification */
        void continueProcessWait(uint32_t id, game::Session& session, RequestLink2 link);

        /** Wait callback.
            Signals the wait result to the UserSide.
            \param id       Wait Id
            \param state    Process state
            \param error    Error */
        void handleWait(uint32_t id, interpreter::Process::State state, interpreter::Error error);

        /*
         *  Request Submission
         */

        /** Post a task to the UserSide.
            This will suspend the specified process.
            The UserTask must eventually call continueProcess() or continueProcessWithFailure().

            This does not report the task or its wait finished. */
        void postNewTask(RequestLink1 link, UserTask* t);

        /** USE WITH CARE: Access the underlying RequestSender.
            You should normally use call() or postNewTask() to talk to the UserSide.
            This method is available as an escape mechanism if you cannot use these.
            Because this obviously lacks the integration with process statuses, it can be only used for quick fire-and-forget tasks.
            \return Sender */
        util::RequestSender<UserSide> sender();

        /** Execute command on user side.
            Synchronously executes the given UserCall.
            This means you can pass parameters into the call and results out of the call using the UserCall object.
            Since user side is not allowed to block, this will not block.
            \param t Command
            \throw interpreter::Error if t.handle() throws */
        void call(UserCall& t);

        /** Execute command on user side, asynchronously.
            Executes the given UserCall without waiting for completion.
            This means you cannot pass parameters by reference, nor can you pass results back; exceptions are swallowed.

            This can be used as a higher-throughput version of call() in places where no results are needed.
            Note that requests are processed in sequence anyway, so even if this call is asynchronously,
            the UserCall will be guaranteed to have been processed before the next call().

            \param t Newly-allocated UserCall descendant. Will become owned by the ScriptSide. */
        void callAsyncNew(UserCall* t);


        /*
         *  Manipulating a running process
         */

        void continueProcess(game::Session& session, RequestLink2 link);
        void joinProcess(game::Session& session, RequestLink2 link, RequestLink2 other);
        void continueProcessWithFailure(game::Session& session, RequestLink2 link, String_t error);
        void detachProcess(game::Session& session, RequestLink2 link);
        void setVariable(game::Session& session, RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value);

        void runProcesses(game::Session& session);


        /*
         *  Setup and shutdown
         */
        void init(game::Session& session);
        void done(game::Session& session);

     private:
        void onProcessGroupFinish(game::Session& session, uint32_t pgid);

        afl::base::SignalConnection conn_processGroupFinish;

        util::RequestSender<UserSide> m_reply;

        struct Wait {
            uint32_t waitId;
            uint32_t processGroupId;
            interpreter::Process* process;
            bool verbose;
            bool hasResult;
            Wait(uint32_t waitId, uint32_t processGroupId, interpreter::Process* process, bool verbose, bool hasResult)
                : waitId(waitId), processGroupId(processGroupId), process(process),
                  verbose(verbose), hasResult(hasResult)
                { }
            Wait()
                : waitId(0), processGroupId(0), process(0), verbose(false), hasResult(false)
                { }
        };
        std::vector<Wait> m_waits;

        bool extractWait(uint32_t pgid, Wait& out);
    };

} }

#endif
