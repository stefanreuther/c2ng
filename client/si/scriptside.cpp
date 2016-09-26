/**
  *  \file client/si/scriptside.cpp
  */

#include "client/si/scriptside.hpp"
#include "client/si/contextprovider.hpp"
#include "client/si/requestlink1.hpp"
#include "client/si/requestlink2.hpp"
#include "client/si/scriptprocedure.hpp"
#include "client/si/userside.hpp"
#include "client/si/usertask.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/values.hpp"

using interpreter::makeBooleanValue;

client::si::ScriptSide::ScriptSide(util::RequestSender<UserSide> reply)
    : conn_processGroupFinish(),
      m_reply(reply),
      m_procedures(),
      m_waits()
{ }

client::si::ScriptSide::~ScriptSide()
{
    for (std::set<ScriptProcedure*>::iterator it = m_procedures.begin(), e = m_procedures.end(); it != e; ++it) {
        (**it).setScriptSide(0);
    }
}

void
client::si::ScriptSide::addProcedure(ScriptProcedure* p)
{
    if (p != 0) {
        m_procedures.insert(p);
    }
}

void
client::si::ScriptSide::removeProcedure(ScriptProcedure* p)
{
    m_procedures.erase(p);
}

void
client::si::ScriptSide::executeCommandWait(uint32_t id, game::Session& session, String_t command, bool verbose, String_t name, std::auto_ptr<ContextProvider> ctxp)
{
    // Log it
    if (verbose) {
        session.log().write(afl::sys::LogListener::Info, "script.input", command);
    }

    // Create process
    interpreter::ProcessList& processList = session.world().processList();
    interpreter::Process& proc = processList.create(session.world(), name);

    // Create BCO
    interpreter::BCORef_t bco = new interpreter::BytecodeObject();
    if (ctxp.get() != 0) {
        ctxp->createContext(session, proc);
    }

    // Create compilation context
    interpreter::MemoryCommandSource mcs(command);
    interpreter::DefaultStatementCompilationContext scc(session.world());
    scc.withContextProvider(&proc);
    scc.withFlag(scc.RefuseBlocks);
    scc.withFlag(scc.LinearExecution);
    if (!verbose) {
        scc.withFlag(scc.ExpressionsAreStatements);
    }

    // Compile
    interpreter::StatementCompiler::StatementResult result;
    try {
        interpreter::StatementCompiler sc(mcs);
        result = sc.compile(*bco, scc);
        sc.finishBCO(*bco, scc);
    }
    catch (std::exception& e) {
        if (interpreter::Error* pe = dynamic_cast<interpreter::Error*>(&e)) {
            session.logError(*pe);
        } else {
            session.logError(interpreter::Error(e.what()));
        }

        // Immediately fail it
        proc.setState(interpreter::Process::Failed);
        handleWait(id, interpreter::Process::Failed, e.what());
        return;
    }

    // Create a wait
    uint32_t pgid = processList.allocateProcessGroup();
    m_waits.push_back(Wait(id, pgid, &proc, verbose, result == interpreter::StatementCompiler::CompiledExpression));

    // Populate process group
    proc.pushFrame(bco, false);
    processList.resumeProcess(proc, pgid);
    processList.startProcessGroup(pgid);

    // Execute
    runProcesses(session);
}

void
client::si::ScriptSide::continueProcessWait(uint32_t id, game::Session& session, RequestLink2 link)
{
    interpreter::ProcessList& list = session.world().processList();
    uint32_t pid;
    if (!link.getProcessId(pid)) {
        handleWait(id, interpreter::Process::Terminated, interpreter::Error(""));
    } else if (interpreter::Process* p = list.getProcessById(pid)) {
        m_waits.push_back(Wait(id, p->getProcessGroupId(), 0, false, false));
        if (link.isWantResult()) {
            p->pushNewValue(0);
        }
        list.continueProcess(*p);
        runProcesses(session);
    } else {
        handleWait(id, interpreter::Process::Terminated, interpreter::Error(""));
    }
}


void
client::si::ScriptSide::handleWait(uint32_t id, interpreter::Process::State state, interpreter::Error error)
{
    class Request : public util::Request<UserSide> {
     public:
        Request(uint32_t id, interpreter::Process::State state, interpreter::Error error)
            : m_id(id), m_state(state), m_error(error)
            { }
        void handle(UserSide& ui)
            { ui.handleWait(m_id, m_state, m_error); }
     private:
        uint32_t m_id;
        interpreter::Process::State m_state;
        interpreter::Error m_error;
    };
    m_reply.postNewRequest(new Request(id, state, error));
}

void
client::si::ScriptSide::postNewTask(RequestLink1 link, UserTask* t)
{
    class Proxy : public util::Request<UserSide> {
     public:
        Proxy(std::auto_ptr<UserTask> t, RequestLink2 link)
            : m_task(t), m_link(link)
            { }
        void handle(UserSide& ui)
            { ui.processTask(*m_task, m_link); }
     private:
        std::auto_ptr<UserTask> m_task;
        RequestLink2 m_link;
    };

    std::auto_ptr<UserTask> tt(t);
    m_reply.postNewRequest(new Proxy(tt, link));
    link.getProcess().suspendForUI();
}

void
client::si::ScriptSide::continueProcess(game::Session& session, RequestLink2 link)
{
    interpreter::ProcessList& list = session.world().processList();
    uint32_t pid;
    if (link.getProcessId(pid)) {
        if (interpreter::Process* p = list.getProcessById(pid)) {
            if (link.isWantResult()) {
                p->pushNewValue(0);
            }
            list.continueProcess(*p);
            runProcesses(session);
        }
    }
}

void
client::si::ScriptSide::joinProcess(game::Session& session, RequestLink2 link, RequestLink2 other)
{
    // FIXME: it is an error if link is invalid but other is valid.
    interpreter::ProcessList& list = session.world().processList();
    uint32_t linkPid, otherPid;
    if (link.getProcessId(linkPid) && other.getProcessId(otherPid)) {
        if (interpreter::Process* p = list.getProcessById(linkPid)) {
            if (interpreter::Process* otherProcess = list.getProcessById(otherPid)) {
                list.joinProcess(*otherProcess, p->getProcessGroupId());
                if (other.isWantResult()) {
                    otherProcess->pushNewValue(0);
                }
            }
        }
    }
}


void
client::si::ScriptSide::continueProcessWithFailure(game::Session& session, RequestLink2 link, String_t error)
{
    interpreter::ProcessList& list = session.world().processList();
    uint32_t pid;
    if (link.getProcessId(pid)) {
        if (interpreter::Process* p = list.getProcessById(pid)) {
            list.continueProcessWithFailure(*p, error);
            runProcesses(session);
        }
    }
}

void
client::si::ScriptSide::detachProcess(game::Session& session, RequestLink2 link)
{
    interpreter::ProcessList& list = session.world().processList();
    uint32_t pid;
    if (link.getProcessId(pid)) {
        if (interpreter::Process* p = list.getProcessById(pid)) {
            Wait w;
            while (extractWait(p->getProcessGroupId(), w)) {
                handleWait(w.waitId, interpreter::Process::Waiting, interpreter::Error(""));
            }
        }
    }
}

void
client::si::ScriptSide::setVariable(game::Session& session, RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value)
{
    interpreter::ProcessList& list = session.world().processList();
    uint32_t pid;
    if (link.getProcessId(pid)) {
        if (interpreter::Process* p = list.getProcessById(pid)) {
            p->setVariable(name, value.get());
        }
    }
}

void
client::si::ScriptSide::runProcesses(game::Session& session)
{
    interpreter::ProcessList& processList = session.world().processList();

    // Run processes. This will execute onProcessGroupFinish() callbacks that process waits.
    processList.run();

    // Remove terminated processes.
    // We cannot remove terminated processes as long as we have active waits;
    // a wait might be interested in a terminated process' state although its process group is not yet finished.
    // FIXME: this means nested UI processes stay around until the surrounding UI process terminated. Can we be smarter?
    if (m_waits.empty()) {
        processList.removeTerminatedProcesses();
    }
}

void
client::si::ScriptSide::init(game::Session& session)
{
    // ex int/if/guiif.cc:initInterpreterGuiInterface

    class Relay : public afl::base::Closure<void(uint32_t)> {
     public:
        Relay(game::Session& session, ScriptSide& self)
            : m_session(session), m_self(self)
            { }
        void call(uint32_t n)
            { m_self.onProcessGroupFinish(m_session, n); }
        Relay* clone() const
            { return new Relay(*this); }
     private:
        game::Session& m_session;
        ScriptSide& m_self;
    };
    conn_processGroupFinish = session.world().processList().sig_processGroupFinish.addNewClosure(new Relay(session, *this));

    // Values
    /* @q System.GUI:Bool (Global Property)
       Graphical interface flag.
       True if PCC is running with graphical interface, False if it is running in console mode. */
    session.world().setNewGlobalValue("SYSTEM.GUI", makeBooleanValue(true));
    // defineGlobalValue("LISTBOX",        new IntSimpleIndexableValue(0, IFListbox,           0));

    // IntExecutionContext::setBreakHandler(checkBreak);
}

void
client::si::ScriptSide::done(game::Session& /*session*/)
{
    conn_processGroupFinish.disconnect();
}

void
client::si::ScriptSide::onProcessGroupFinish(game::Session& session, uint32_t pgid)
{
    Wait w;
    while (extractWait(pgid, w)) {
        // Report state
        if (w.process != 0) {
            interpreter::Error e = w.process->getError();
            switch (w.process->getState()) {
             case interpreter::Process::Suspended:
                if (w.verbose) {
                    session.log().write(afl::sys::LogListener::Info, "script.state", session.translator().translateString("Suspended."));
                }
                break;
             case interpreter::Process::Frozen:
                if (w.verbose) {
                    session.log().write(afl::sys::LogListener::Info, "script.state", session.translator().translateString("Frozen."));
                }
                break;
             case interpreter::Process::Runnable:
             case interpreter::Process::Running:
             case interpreter::Process::Waiting:
                break;
             case interpreter::Process::Ended:
                if (w.verbose && w.hasResult) {
                    afl::data::Value* p = w.process->getResult();
                    if (p == 0) {
                        session.log().write(afl::sys::LogListener::Info, "script.empty", "Empty");
                    } else {
                        session.log().write(afl::sys::LogListener::Info, "script.result", interpreter::toString(p, true));
                    }
                }
                break;
             case interpreter::Process::Terminated:
                // Terminated, i.e. "End" statement. Log only when user specified an expression,
                // to tell them why they don't get a result.
                if (w.verbose && w.hasResult) {
                    session.log().write(afl::sys::LogListener::Info, "script.state", session.translator().translateString("Terminated."));
                }
                break;

             case interpreter::Process::Failed:
                session.logError(e);
                break;
            }
            handleWait(w.waitId, w.process->getState(), e);
        } else {
            handleWait(w.waitId, interpreter::Process::Terminated, interpreter::Error(""));
        }
    }
    session.notifyListeners();
}

bool
client::si::ScriptSide::extractWait(uint32_t pgid, Wait& out)
{
    for (std::vector<Wait>::iterator it = m_waits.begin(); it != m_waits.end(); ++it) {
        if (it->processGroupId == pgid) {
            // Copy/remove the wait
            out = *it;
            m_waits.erase(it);
            return true;
        }
    }
    return false;
}
