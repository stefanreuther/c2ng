/**
  *  \file game/proxy/processlistproxy.cpp
  *  \brief Class game::proxy::ProcessListProxy
  */

#include <memory>
#include "game/proxy/processlistproxy.hpp"
#include "util/slaveobject.hpp"

using game::interface::ProcessListEditor;

class game::proxy::ProcessListProxy::Trampoline : public util::SlaveObject<Session> {
 public:
    Trampoline(util::RequestSender<ProcessListProxy> reply)
        : m_reply(reply),
          m_editor()
        { }

    ~Trampoline()
        { }

    virtual void init(Session& session)
        {
            m_editor.reset(new ProcessListEditor(session.processList()));
        }

    virtual void done(Session& /*session*/)
        { }

    void buildResult(Session& session, Infos_t& out)
        {
            if (m_editor.get() != 0) {
                for (size_t i = 0, n = m_editor->getNumProcesses(); i < n; ++i) {
                    Info_t info;
                    if (m_editor->describe(i, info, session.notifications(), session.translator())) {
                        out.push_back(info);
                    }
                }
            }
        }

    void sendUpdate(Session& session)
        {
            class Update : public util::Request<ProcessListProxy> {
             public:
                Update(Session& session, Trampoline& tpl)
                    : m_infos()
                    { tpl.buildResult(session, m_infos); }
                virtual void handle(ProcessListProxy& proxy)
                    { proxy.sig_listChange.raise(m_infos); }
             private:
                Infos_t m_infos;
            };
            m_reply.postNewRequest(new Update(session, *this));
        }

    void setProcessState(Session& session, uint32_t pid, State_t state)
        {
            if (m_editor.get() != 0) {
                m_editor->setProcessState(pid, state);
                sendUpdate(session);
            }
        }

    void setAllProcessState(Session& session, State_t state)
        {
            if (m_editor.get() != 0) {
                m_editor->setAllProcessState(state);
                sendUpdate(session);
            }
        }

    void setProcessPriority(Session& session, uint32_t pid, int pri)
        {
            if (m_editor.get() != 0) {
                m_editor->setProcessPriority(pid, pri);
                sendUpdate(session);
            }
        }

    uint32_t commit(Session& session)
        {
            uint32_t pgid = session.processList().allocateProcessGroup();
            if (m_editor.get() != 0) {
                m_editor->commit(pgid);
            }
            return pgid;
        }

 private:
    util::RequestSender<ProcessListProxy> m_reply;
    std::auto_ptr<ProcessListEditor> m_editor;
};


// Constructor.
game::proxy::ProcessListProxy::ProcessListProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender, new Trampoline(m_reply.getSender()))
{ }

// Destructor.
game::proxy::ProcessListProxy::~ProcessListProxy()
{ }

// Initialize and retrieve initial process list.
void
game::proxy::ProcessListProxy::init(WaitIndicator& link, Infos_t& result)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(Infos_t& result)
            : m_result(result)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.buildResult(session, m_result); }
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
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(uint32_t pid, State_t state)
            : m_pid(pid), m_state(state)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setProcessState(session, m_pid, m_state); }
     private:
        uint32_t m_pid;
        State_t m_state;
    };
    m_request.postNewRequest(new Request(pid, state));
}

// Prepare a state change for all processes.
void
game::proxy::ProcessListProxy::setAllProcessState(State_t state)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(State_t state)
            : m_state(state)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setAllProcessState(session, m_state); }
     private:
        State_t m_state;
    };
    m_request.postNewRequest(new Request(state));
}

// Set process priority.
void
game::proxy::ProcessListProxy::setProcessPriority(uint32_t pid, int pri)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request(uint32_t pid, int pri)
            : m_pid(pid), m_pri(pri)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setProcessPriority(session, m_pid, m_pri); }
     private:
        uint32_t m_pid;
        int m_pri;
    };
    m_request.postNewRequest(new Request(pid, pri));
}

// Perform all prepared state changes.
uint32_t
game::proxy::ProcessListProxy::commit(WaitIndicator& link)
{
    class Request : public util::SlaveRequest<Session, Trampoline> {
     public:
        Request()
            : m_result()
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { m_result = tpl.commit(session); }
        uint32_t get() const
            { return m_result; }
     private:
        uint32_t m_result;
    };
    Request req;
    link.call(m_request, req);
    return req.get();
}
