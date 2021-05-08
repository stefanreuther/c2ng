/**
  *  \file game/proxy/taskeditorproxy.cpp
  *  \brief Class game::proxy::TaskEditorProxy
  */

#include "game/proxy/taskeditorproxy.hpp"
#include "afl/base/signalconnection.hpp"

using interpreter::Process;


/*
 *  Trampoline
 */

class game::proxy::TaskEditorProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<TaskEditorProxy> reply)
        : m_session(session),
          m_reply(reply),
          m_editor()
        { }

    ~Trampoline()
        {
            // Explicitly deselect the auto-task.
            // This causes it to be scheduled to run.
            selectTask(0, Process::pkDefault, false);
        }

    void selectTask(Id_t id, Process::ProcessKind kind, bool create);

    void setCursor(size_t newCursor);

    void describe(Status& out) const;

    void sendStatus();

 private:
    Session& m_session;
    util::RequestSender<TaskEditorProxy> m_reply;
    afl::base::Ptr<interpreter::TaskEditor> m_editor;
    afl::base::SignalConnection conn_change;
};

void
game::proxy::TaskEditorProxy::Trampoline::selectTask(Id_t id, Process::ProcessKind kind, bool create)
{
    // Remember the old editor
    // This means the old one will die no earlier than releaseAutoTaskEditor() below.
    // In particular, when this function is called with the same parameters again, it'll re-use the same instance.
    afl::base::Ptr<interpreter::TaskEditor> old = m_editor;

    // Disconnect the signal. Anything that happens during the change will be ignored,
    // we explicitly send a status at the end.
    conn_change.disconnect();

    // Set up new one
    m_editor = m_session.getAutoTaskEditor(id, kind, create);

    // Destroy old one
    m_session.releaseAutoTaskEditor(old);

    // Connect the signal and inform user
    if (m_editor.get() != 0) {
        conn_change = m_editor->sig_change.add(this, &Trampoline::sendStatus);
    }
    sendStatus();
}

void
game::proxy::TaskEditorProxy::Trampoline::setCursor(size_t newCursor)
{
    if (m_editor.get() != 0) {
        m_editor->setCursor(newCursor);
    }
}


void
game::proxy::TaskEditorProxy::Trampoline::describe(Status& out) const
{
    out.commands.clear();
    if (m_editor.get() != 0) {
        m_editor->getAll(out.commands);
        out.pc                 = m_editor->getPC();
        out.cursor             = m_editor->getCursor();
        out.isInSubroutineCall = m_editor->isInSubroutineCall();
        out.valid              = true;
    } else {
        out.pc                 = 0;
        out.cursor             = 0;
        out.isInSubroutineCall = false;
        out.valid              = false;
    }
}

void
game::proxy::TaskEditorProxy::Trampoline::sendStatus()
{
    class Task : public util::Request<TaskEditorProxy> {
     public:
        Task(const Trampoline& self)
            : m_status()
            { self.describe(m_status); }
        virtual void handle(TaskEditorProxy& proxy)
            { proxy.sig_change.raise(m_status); }
     private:
        Status m_status;
    };
    m_reply.postNewRequest(new Task(*this));
}



class game::proxy::TaskEditorProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<TaskEditorProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<TaskEditorProxy> m_reply;
};


/*
 *  TaskEditorProxy
 */

game::proxy::TaskEditorProxy::TaskEditorProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

game::proxy::TaskEditorProxy::~TaskEditorProxy()
{ }

void
game::proxy::TaskEditorProxy::selectTask(Id_t id, interpreter::Process::ProcessKind kind, bool create)
{
    m_trampoline.postRequest(&Trampoline::selectTask, id, kind, create);
}

void
game::proxy::TaskEditorProxy::setCursor(size_t newCursor)
{
    m_trampoline.postRequest(&Trampoline::setCursor, newCursor);
}
