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

namespace client { namespace si {

    class ContextProvider;
    class RequestLink1;
    class RequestLink2;
    class ScriptProcedure;
    class UserSide;
    class UserTask;

    class ScriptSide : public util::SlaveObject<game::Session> {
     public:
        ScriptSide(util::RequestSender<UserSide> reply);
        ~ScriptSide();

        // Add/remove procedures
        void addProcedure(ScriptProcedure* p);
        void removeProcedure(ScriptProcedure* p);

        // Starting new stuff
        // --> executeCommandWait(id):
        //     - execute the given stuff
        //     - if a result is known, call handleWait(id, state, error)
        void executeCommandWait(uint32_t id, game::Session& session, String_t command, bool verbose, String_t name, std::auto_ptr<ContextProvider> ctxp);
        void continueProcessWait(uint32_t id, game::Session& session, RequestLink2 link);
        void handleWait(uint32_t id, interpreter::Process::State state, interpreter::Error error);

        // --> postNewTask()
        //     - suspend current process
        //     - must eventually call continueProcess() or continueProcessWithFailure()
        //     - does not report current task to report finish
        void postNewTask(RequestLink1 link, UserTask* t);

        void continueProcess(game::Session& session, RequestLink2 link);
        void joinProcess(game::Session& session, RequestLink2 link, RequestLink2 other);
        void continueProcessWithFailure(game::Session& session, RequestLink2 link, String_t error);
        void detachProcess(game::Session& session, RequestLink2 link);
        void setVariable(game::Session& session, RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value);

        void runProcesses(game::Session& session);

        // SlaveObject:
        virtual void init(game::Session& session);
        virtual void done(game::Session& session);

     private:
        void onProcessGroupFinish(game::Session& session, uint32_t pgid);

        afl::base::SignalConnection conn_processGroupFinish;

        util::RequestSender<UserSide> m_reply;
        std::set<ScriptProcedure*> m_procedures;

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
