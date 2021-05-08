/**
  *  \file game/proxy/processlistproxy.cpp
  *  \brief Class game::proxy::ProcessListProxy
  */

#include "game/proxy/processlistproxy.hpp"

using game::interface::ProcessListEditor;

class game::proxy::ProcessListProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<ProcessListProxy> reply)
        : m_session(session),
          m_reply(reply),
          m_editor(session.processList())
        { }

    ~Trampoline()
        { }

    void buildResult(Infos_t& out)
        {
            for (size_t i = 0, n = m_editor.getNumProcesses(); i < n; ++i) {
                Info_t info;
                if (m_editor.describe(i, info, m_session.notifications(), m_session.translator())) {
                    out.push_back(info);
                }
            }
        }

    void sendUpdate()
        {
            class Update : public util::Request<ProcessListProxy> {
             public:
                Update(Trampoline& tpl)
                    : m_infos()
                    { tpl.buildResult(m_infos); }
                virtual void handle(ProcessListProxy& proxy)
                    { proxy.sig_listChange.raise(m_infos); }
             private:
                Infos_t m_infos;
            };
            m_reply.postNewRequest(new Update(*this));
        }

    void setProcessState(uint32_t pid, State_t state)
        {
            m_editor.setProcessState(pid, state);
            sendUpdate();
        }

    void setAllProcessState(State_t state)
        {
            m_editor.setAllProcessState(state);
            sendUpdate();
        }

    void setProcessPriority(uint32_t pid, int pri)
        {
            m_editor.setProcessPriority(pid, pri);
            sendUpdate();
        }

    void resumeConfirmedProcesses()
        {
            m_session.notifications().resumeConfirmedProcesses(m_editor);
            sendUpdate();
        }

    uint32_t commit()
        {
            uint32_t pgid = m_session.processList().allocateProcessGroup();
            m_editor.commit(pgid);
            return pgid;
        }

 private:
    Session& m_session;
    util::RequestSender<ProcessListProxy> m_reply;
    ProcessListEditor m_editor;
};


class game::proxy::ProcessListProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<ProcessListProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<ProcessListProxy> m_reply;
};


// Constructor.
game::proxy::ProcessListProxy::ProcessListProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

// Destructor.
game::proxy::ProcessListProxy::~ProcessListProxy()
{ }

// Initialize and retrieve initial process list.
void
game::proxy::ProcessListProxy::init(WaitIndicator& link, Infos_t& result)
{
    class Request : public util::Request<Trampoline> {
     public:
        Request(Infos_t& result)
            : m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.buildResult(m_result); }
     private:
        Infos_t& m_result;
    };
    Request req(result);
    link.call(m_request, req);
}

// Prepare a state change.
void
game::proxy::ProcessListProxy::setProcessState(uint32_t pid, State_t state)
{
    m_request.postRequest(&Trampoline::setProcessState, pid, state);
}

// Prepare a state change for all processes.
void
game::proxy::ProcessListProxy::setAllProcessState(State_t state)
{
    m_request.postRequest(&Trampoline::setAllProcessState, state);
}

// Set process priority.
void
game::proxy::ProcessListProxy::setProcessPriority(uint32_t pid, int pri)
{
    m_request.postRequest(&Trampoline::setProcessPriority, pid, pri);
}

// Resume confirmed processes.
void
game::proxy::ProcessListProxy::resumeConfirmedProcesses()
{
    m_request.postRequest(&Trampoline::resumeConfirmedProcesses);
}

// Perform all prepared state changes.
uint32_t
game::proxy::ProcessListProxy::commit(WaitIndicator& link)
{
    class Request : public util::Request<Trampoline> {
     public:
        Request()
            : m_result()
            { }
        virtual void handle(Trampoline& tpl)
            { m_result = tpl.commit(); }
        uint32_t get() const
            { return m_result; }
     private:
        uint32_t m_result;
    };
    Request req;
    link.call(m_request, req);
    return req.get();
}
