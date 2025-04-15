/**
  *  \file client/si/scriptside.cpp
  *  \brief Class client::si::ScriptSide
  */

#include "client/si/scriptside.hpp"
#include "afl/sys/semaphore.hpp"
#include "client/si/requestlink1.hpp"
#include "client/si/requestlink2.hpp"
#include "client/si/userside.hpp"
#include "client/si/usertask.hpp"

namespace {
    const char*const LOG_NAME = "script.si";
}

// Constructor.
client::si::ScriptSide::ScriptSide(util::RequestSender<UserSide> reply, game::Session& session)
    : m_session(session),
      conn_processGroupFinish(),
      m_reply(reply),
      m_waits()
{
    conn_processGroupFinish = session.processList().sig_processGroupFinish.add(this, &ScriptSide::onProcessGroupFinish);
    session.setNewScriptRunner(afl::base::Closure<void()>::makeBound(this, &ScriptSide::runProcesses));
}

// Destructor.
client::si::ScriptSide::~ScriptSide()
{
    m_session.setNewScriptRunner(0);
}

// Access the underlying RequestSender.
util::RequestSender<client::si::UserSide>
client::si::ScriptSide::sender()
{
    return m_reply;
}


/*
 *  Starting new processes
 */

// Execute a script-based task.
void
client::si::ScriptSide::executeTaskWait(uint32_t waitId, std::auto_ptr<ScriptTask> task)
{
    // Populate process group
    interpreter::ProcessList& processList = m_session.processList();
    const uint32_t pgid = processList.allocateProcessGroup();
    task->execute(pgid, m_session);

    // Run it
    m_waits.push_back(Wait(waitId, pgid));
    processList.startProcessGroup(pgid);
    runProcesses();
}

// Continue a detached process.
void
client::si::ScriptSide::continueProcessWait(uint32_t waitId, RequestLink2 link)
{
    interpreter::ProcessList& list = m_session.processList();
    uint32_t pid;
    if (!link.getProcessId(pid)) {
        // Null link > signal completion immediately
        onTaskComplete(waitId);
    } else if (interpreter::Process* p = list.findProcessById(pid)) {
        // Valid link > run it normally
        m_waits.push_back(Wait(waitId, p->getProcessGroupId()));
        if (link.isWantResult()) {
            p->pushNewValue(0);
        }
        list.continueProcess(*p);
        runProcesses();
    } else {
        // Link to dead process > signal completion immediately
        onTaskComplete(waitId);
    }
}


/*
 *  Request Submission
 */

// Post a task to the UserSide.
void
client::si::ScriptSide::postNewTask(RequestLink1 link, UserTask* t)
{
    // Adaptor to wrap the UserTask/RequestLink1 into a Request;
    // executed as interaction on UserSide.
    class Proxy : public util::Request<UserSide> {
     public:
        Proxy(std::auto_ptr<UserTask> t, RequestLink2 link)
            : m_task(t), m_link(link)
            { }
        void handle(UserSide& us)
            {
                if (Control* ctl = us.getControl()) {
                    try {
                        m_task->handle(*ctl, m_link);
                    }
                    catch (std::exception& e) {
                        us.continueProcessWithFailure(m_link, e.what());
                    }
                } else {
                    us.continueProcessWithFailure(m_link, interpreter::Error::contextError().what());
                }
            }
     private:
        std::auto_ptr<UserTask> m_task;
        RequestLink2 m_link;
    };

    // Notify listeners to keep UI up-to-date
    m_session.notifyListeners();

    // Post request
    std::auto_ptr<UserTask> tt(t);
    postNewInteraction(new Proxy(tt, link));

    // Mark process suspended (afterwards, so if the above throws, the request will be properly aborted)
    link.getProcess().suspendForUI();
}

// Post an interaction request to the UserSide.
void
client::si::ScriptSide::postNewInteraction(util::Request<UserSide>* req)
{
    class Adaptor : public util::Request<UserSide> {
     public:
        Adaptor(std::auto_ptr<util::Request<UserSide> > t)
            : m_task(t)
            { }
        void handle(UserSide& us)
            { us.processInteraction(*m_task); }
     private:
        std::auto_ptr<util::Request<UserSide> > m_task;
    };
    std::auto_ptr<util::Request<UserSide> > tt(req);
    m_reply.postNewRequest(new Adaptor(tt));
}

// Execute command on UserSide.
void
client::si::ScriptSide::call(util::Request<Control>& t)
{
    class Proxy : public util::Request<UserSide> {
     public:
        Proxy(util::Request<Control>& t, afl::sys::Semaphore& result, std::auto_ptr<interpreter::Error>& error)
            : m_task(t),
              m_result(result),
              m_error(error)
            { }
        ~Proxy()
            { m_result.post(); }
        void handle(UserSide& ui)
            {
                try {
                    ui.processCall(m_task);
                }
                catch (interpreter::Error& e) {
                    m_error.reset(new interpreter::Error(e));
                }
                catch (std::exception& e) {
                    m_error.reset(new interpreter::Error(e.what()));
                }
            }
     private:
        util::Request<Control>& m_task;
        afl::sys::Semaphore& m_result;
        std::auto_ptr<interpreter::Error>& m_error;
    };

    afl::sys::Semaphore result(0);
    std::auto_ptr<interpreter::Error> error;
    m_reply.postNewRequest(new Proxy(t, result, error));
    result.wait();
    if (error.get() != 0) {
        throw *error;
    }
}

// Execute command on UserSide, asynchronously.
void
client::si::ScriptSide::callAsyncNew(util::Request<Control>* t)
{
    class Proxy : public util::Request<UserSide> {
     public:
        Proxy(std::auto_ptr<util::Request<Control> >& t)
            : m_task(t)
            { }
        ~Proxy()
            { }
        void handle(UserSide& us)
            {
                try {
                    us.processCall(*m_task);
                }
                catch (std::exception& e) {
                    us.mainLog().write(afl::sys::LogListener::Error, LOG_NAME, us.translator()("Error in user-interface thread"), e);
                }
            }
     private:
        std::auto_ptr<util::Request<Control> > m_task;
    };

    std::auto_ptr<util::Request<Control> > tt(t);
    m_reply.postNewRequest(new Proxy(tt));
}


/*
 *  Manipulating a running process
 */

// Manipulating a running process.
void
client::si::ScriptSide::continueProcess(RequestLink2 link)
{
    interpreter::ProcessList& list = m_session.processList();
    uint32_t pid;
    if (link.getProcessId(pid)) {
        if (interpreter::Process* p = list.findProcessById(pid)) {
            if (link.isWantResult()) {
                p->pushNewValue(0);
            }
            list.continueProcess(*p);
            runProcesses();
        }
    }
}

// Join processes into a process group.
void
client::si::ScriptSide::joinProcess(RequestLink2 link, RequestLink2 other)
{
    // FIXME: it is an error if link is invalid but other is valid.
    interpreter::ProcessList& list = m_session.processList();
    uint32_t linkPid, otherPid;
    if (link.getProcessId(linkPid) && other.getProcessId(otherPid)) {
        if (interpreter::Process* p = list.findProcessById(linkPid)) {
            if (interpreter::Process* otherProcess = list.findProcessById(otherPid)) {
                list.joinProcess(*otherProcess, p->getProcessGroupId());
                if (other.isWantResult()) {
                    otherProcess->pushNewValue(0);
                }
            }
        }
    }
}

// Join process group.
void
client::si::ScriptSide::joinProcessGroup(RequestLink2 link, uint32_t oldGroup)
{
    // It is an error if link is invalid
    interpreter::ProcessList& list = m_session.processList();
    uint32_t linkPid;
    if (link.getProcessId(linkPid)) {
        if (interpreter::Process* p = list.findProcessById(linkPid)) {
            list.joinProcessGroup(oldGroup, p->getProcessGroupId());
        }
    }
}

// Continue process with an error.
void
client::si::ScriptSide::continueProcessWithFailure(RequestLink2 link, String_t error)
{
    interpreter::ProcessList& list = m_session.processList();
    uint32_t pid;
    if (link.getProcessId(pid)) {
        if (interpreter::Process* p = list.findProcessById(pid)) {
            list.continueProcessWithFailure(*p, error);
            runProcesses();
        }
    }
}

// Detach process.
void
client::si::ScriptSide::detachProcess(RequestLink2 link)
{
    interpreter::ProcessList& list = m_session.processList();
    uint32_t pid;
    if (link.getProcessId(pid)) {
        if (interpreter::Process* p = list.findProcessById(pid)) {
            Wait w;
            while (extractWait(p->getProcessGroupId(), w)) {
                onTaskComplete(w.waitId);
            }
        }
    }
}

// Set variable in a process.
void
client::si::ScriptSide::setVariable(RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value)
{
    interpreter::ProcessList& list = m_session.processList();
    uint32_t pid;
    if (link.getProcessId(pid)) {
        if (interpreter::Process* p = list.findProcessById(pid)) {
            p->setVariable(name, value.get());
        }
    }
}


/*
 *  Running Processes
 */

// Run processes.
void
client::si::ScriptSide::runProcesses()
{
    interpreter::ProcessList& processList = m_session.processList();

    // Run processes. This will execute onProcessGroupFinish() callbacks that process waits.
    // TODO: add break handling here
    processList.run(0);
    processList.removeTerminatedProcesses();

    // Clean up messages
    m_session.notifications().removeOrphanedMessages();
}


/*
 *  Private
 */

// Wait callback.
void
client::si::ScriptSide::onTaskComplete(uint32_t waitId)
{
    m_reply.postRequest(&UserSide::onTaskComplete, waitId);
}

// Process group completion callback.
void
client::si::ScriptSide::onProcessGroupFinish(uint32_t pgid)
{
    // Signal everyone who waits on us
    // (Should only be one, but supporting multiple isn't hard here.)
    Wait w;
    while (extractWait(pgid, w)) {
        onTaskComplete(w.waitId);
    }

    // Notify listeners about changes by the terminated process group
    m_session.notifyListeners();

    // Caller is (indirectly) runProcesses() who will clean up.
}

// Look up a wait for a process group.
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
