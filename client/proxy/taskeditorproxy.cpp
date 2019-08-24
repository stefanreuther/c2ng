/**
  *  \file client/proxy/taskeditorproxy.cpp
  */

#include "client/proxy/taskeditorproxy.hpp"
#include "afl/base/signalconnection.hpp"
#include "util/slaveobject.hpp"

using interpreter::Process;


/*
 *  Trampoline
 */

class client::proxy::TaskEditorProxy::Trampoline : public util::SlaveObject<game::Session> {
 public:
    Trampoline(util::RequestSender<TaskEditorProxy> reply)
        : m_reply(reply),
          m_editor()
        { }

    ~Trampoline()
        { }

    virtual void init(game::Session&)
        { }

    virtual void done(game::Session& session)
        {
            // Explicitly deselect the auto-task.
            // This causes it to be scheduled to run.
            selectTask(session, 0, Process::pkDefault, false);
        }

    void selectTask(game::Session& session, game::Id_t id, Process::ProcessKind kind, bool create)
        {
            // Remember the old editor
            // This means the old one will die no earlier than releaseAutoTaskEditor() below.
            // In particular, when this function is called with the same parameters again, it'll re-use the same instance.
            afl::base::Ptr<interpreter::TaskEditor> old = m_editor;

            // Disconnect the signal. Anything that happens during the change will be ignored,
            // we explicitly send a status at the end.
            conn_change.disconnect();

            // Set up new one
            m_editor = session.getAutoTaskEditor(id, kind, create);

            // Destroy old one
            session.releaseAutoTaskEditor(old);

            // Connect the signal and inform user
            if (m_editor.get() != 0) {
                conn_change = m_editor->sig_change.add(this, &Trampoline::sendStatus);
            }
            sendStatus();
        }

    interpreter::TaskEditor* get() const
        { return m_editor.get(); }

    void describe(Status& out) const;

    void sendStatus();

 private:
    util::RequestSender<TaskEditorProxy> m_reply;
    afl::base::Ptr<interpreter::TaskEditor> m_editor;
    afl::base::SignalConnection conn_change;
};

void
client::proxy::TaskEditorProxy::Trampoline::describe(Status& out) const
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
client::proxy::TaskEditorProxy::Trampoline::sendStatus()
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


/*
 *  TaskEditorProxy
 */

client::proxy::TaskEditorProxy::TaskEditorProxy(util::RequestDispatcher& reply, util::RequestSender<game::Session> gameSender)
    : m_reply(reply, *this),
      m_trampoline(gameSender, new Trampoline(m_reply.getSender()))
{ }

client::proxy::TaskEditorProxy::~TaskEditorProxy()
{ }

void
client::proxy::TaskEditorProxy::selectTask(game::Id_t id, interpreter::Process::ProcessKind kind, bool create)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(game::Id_t id, Process::ProcessKind kind, bool create)
            : m_id(id), m_kind(kind), m_create(create)
            { }

        virtual void handle(game::Session& session, Trampoline& tpl)
            { tpl.selectTask(session, m_id, m_kind, m_create); }

     private:
        game::Id_t m_id;
        Process::ProcessKind m_kind;
        bool m_create;
    };
    m_trampoline.postNewRequest(new Task(id, kind, create));
}

void
client::proxy::TaskEditorProxy::setCursor(size_t newCursor)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(size_t cursor)
            : m_cursor(cursor)
            { }

        virtual void handle(game::Session& /*session*/, Trampoline& tpl)
            {
                if (interpreter::TaskEditor* p = tpl.get()) {
                    p->setCursor(m_cursor);
                }
            }

     private:
        size_t m_cursor;
    };
    m_trampoline.postNewRequest(new Task(newCursor));
}
