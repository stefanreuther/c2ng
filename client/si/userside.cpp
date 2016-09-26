/**
  *  \file client/si/userside.cpp
  *  \brief Class client::si::UserSide
  */

#include "client/si/userside.hpp"
#include "client/si/control.hpp"
#include "client/si/requestlink1.hpp"
#include "client/si/requestlink2.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/usertask.hpp"
#include "afl/string/format.hpp"

// Constructor.
client::si::UserSide::UserSide(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& self)
    : m_gameSender(gameSender),
      m_receiver(self, *this),
      m_slave(gameSender, new ScriptSide(m_receiver.getSender())),
      m_waitIdCounter(0)
{ }

// Destructor.
client::si::UserSide::~UserSide()
{ }

// Post a request to execute on the ScriptSide.
void
client::si::UserSide::postNewRequest(util::SlaveRequest<game::Session, ScriptSide>* request)
{
    m_slave.postNewRequest(request);
}

// Continue a process after UI callout.
void
client::si::UserSide::continueProcess(RequestLink2 link)
{
    class Task : public util::SlaveRequest<game::Session,ScriptSide> {
     public:
        Task(RequestLink2 link)
            : m_link(link)
            { }
        void handle(game::Session& session, ScriptSide& t)
            { t.continueProcess(session, m_link); }
     private:
        RequestLink2 m_link;
    };
    m_slave.postNewRequest(new Task(link));
}

void
client::si::UserSide::joinProcess(RequestLink2 link, RequestLink2 other)
{
    class Task : public util::SlaveRequest<game::Session,ScriptSide> {
     public:
        Task(RequestLink2 link, RequestLink2 other)
            : m_link(link),
              m_other(other)
            { }
        void handle(game::Session& session, ScriptSide& t)
            { t.joinProcess(session, m_link, m_other); }
     private:
        RequestLink2 m_link;
        RequestLink2 m_other;
    };
    if (other.isValid()) {
        m_slave.postNewRequest(new Task(link, other));
    }
}


// Continue a process after UI callout with error.
void
client::si::UserSide::continueProcessWithFailure(RequestLink2 link, String_t error)
{
    class Task : public util::SlaveRequest<game::Session,ScriptSide> {
     public:
        Task(RequestLink2 link, String_t error)
            : m_link(link),
              m_error(error)
            { }
        void handle(game::Session& session, ScriptSide& t)
            { t.continueProcessWithFailure(session, m_link, m_error); }
     private:
        RequestLink2 m_link;
        String_t m_error;
    };
    m_slave.postNewRequest(new Task(link, error));
}

// Detach from process after UI callout.
void
client::si::UserSide::detachProcess(RequestLink2 link)
{
    class Task : public util::SlaveRequest<game::Session, ScriptSide> {
     public:
        Task(RequestLink2 link)
            : m_link(link)
            { }
        void handle(game::Session& session, ScriptSide& t)
            { t.detachProcess(session, m_link); }
     private:
        RequestLink2 m_link;
    };
    m_slave.postNewRequest(new Task(link));
}

// Process a UserTask.
void
client::si::UserSide::processTask(UserTask& t, RequestLink2 link)
{
    if (m_controls.empty()) {
        continueProcessWithFailure(link, interpreter::Error::contextError().what());
    } else {
        Control& ctl = *m_controls.back();
        ctl.setInteracting(true);
        try {
            t.handle(*this, ctl, link);
        }
        catch (std::exception& e) {
            continueProcessWithFailure(link, e.what());
        }
        ctl.setInteracting(false);
    }
}

// Set variable in process.
void
client::si::UserSide::setVariable(RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value)
{
    class Task : public util::SlaveRequest<game::Session,ScriptSide> {
     public:
        Task(RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value)
            : m_link(link),
              m_name(name),
              m_value(value)
            { }
        void handle(game::Session& session, ScriptSide& t)
            { t.setVariable(session, m_link, m_name, m_value); }
     private:
        RequestLink2 m_link;
        String_t m_name;
        std::auto_ptr<afl::data::Value> m_value;
    };
    m_slave.postNewRequest(new Task(link, name, value));
}

// Allocate a wait Id.
uint32_t
client::si::UserSide::allocateWaitId()
{
    return ++m_waitIdCounter;
}

// Continue a detached process and setup wait.
void
client::si::UserSide::continueProcessWait(uint32_t id, RequestLink2 link)
{
    class ContinueTask : public util::SlaveRequest<game::Session, ScriptSide> {
     public:
        ContinueTask(uint32_t id, RequestLink2 link)
            : m_id(id),
              m_link(link)
            { }
        void handle(game::Session& s, ScriptSide& t)
            { t.continueProcessWait(m_id, s, m_link); }
     private:
        uint32_t m_id;
        RequestLink2 m_link;
    };
    m_slave.postNewRequest(new ContinueTask(id, link));
}

// Execute a command and setup wait.
void
client::si::UserSide::executeCommandWait(uint32_t id, String_t command, bool verbose, String_t name, std::auto_ptr<ContextProvider> ctxp)
{
    class StartTask : public util::SlaveRequest<game::Session, ScriptSide> {
     public:
        StartTask(uint32_t id, String_t cmd, bool verbose, String_t name, std::auto_ptr<ContextProvider> ctxp)
            : m_id(id),
              m_command(cmd),
              m_verbose(verbose),
              m_name(name),
              m_contextProvider(ctxp)
            { }
        void handle(game::Session& s, ScriptSide& t)
            { t.executeCommandWait(m_id, s, m_command, m_verbose, m_name, m_contextProvider); }
     private:
        uint32_t m_id;
        String_t m_command;
        bool m_verbose;
        String_t m_name;
        std::auto_ptr<ContextProvider> m_contextProvider;
    };
    m_slave.postNewRequest(new StartTask(id, command, verbose, name, ctxp));
}

// Execute a key command and setup wait.
void
client::si::UserSide::executeKeyCommandWait(uint32_t id, String_t keymapName, util::Key_t key, int prefix, std::auto_ptr<ContextProvider> ctxp)
{
    class StartTask : public util::SlaveRequest<game::Session, ScriptSide> {
     public:
        StartTask(uint32_t id, String_t keymapName, util::Key_t key, int prefix, std::auto_ptr<ContextProvider> ctxp)
            : m_id(id),
              m_keymapName(keymapName),
              m_key(key),
              m_prefix(prefix),
              m_contextProvider(ctxp)
            { }
        void handle(game::Session& s, ScriptSide& t)
            {
                util::KeymapRef_t k = s.world().keymaps().getKeymapByName(m_keymapName);
                util::Atom_t a = (k != 0 ? k->lookupCommand(m_key) : 0);
                if (a != 0) {
                    t.executeCommandWait(m_id, s, afl::string::Format("C2$Eval %d, %d", a, m_prefix), false,
                                         afl::string::Format(s.translator().translate("Key '%s' in '%s'").c_str(), util::formatKey(m_key), m_keymapName),
                                         m_contextProvider);
                } else {
                    t.handleWait(m_id, interpreter::Process::Terminated, interpreter::Error(""));
                }
            }
     private:
        uint32_t m_id;
        String_t m_keymapName;
        util::Key_t m_key;
        int m_prefix;
        std::auto_ptr<ContextProvider> m_contextProvider;
    };
    m_slave.postNewRequest(new StartTask(id, keymapName, key, prefix, ctxp));
}

// Handle successful wait.
void
client::si::UserSide::handleWait(uint32_t id, interpreter::Process::State state, interpreter::Error error)
{
    for (size_t i = m_controls.size(); i > 0; --i) {
        m_controls[i-1]->handleWait(id, state, error);
    }
}

// Add listener.
void
client::si::UserSide::addControl(Control& p)
{
    m_controls.push_back(&p);
}

// Remove listener.
void
client::si::UserSide::removeControl(Control& p)
{
    for (std::vector<Control*>::iterator it = m_controls.begin(); it != m_controls.end(); ++it) {
        if (*it == &p) {
            m_controls.erase(it);
            break;
        }
    }
}
