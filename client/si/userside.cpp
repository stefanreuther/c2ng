/**
  *  \file client/si/userside.cpp
  *  \brief Class client::si::UserSide
  */

#include "client/si/userside.hpp"
#include "afl/string/format.hpp"
#include "client/si/control.hpp"
#include "client/si/scriptside.hpp"
#include "client/si/usercall.hpp"
#include "game/extraidentifier.hpp"

const size_t SCREEN_HISTORY_SIZE = 50;

const game::ExtraIdentifier<game::Session, client::si::ScriptSide> client::si::SCRIPTSIDE_ID = {{}};

// Constructor.
client::si::UserSide::UserSide(ui::Root& root,
                               util::RequestSender<game::Session> gameSender,
                               afl::string::Translator& tx,
                               util::RequestDispatcher& self,
                               util::MessageCollector& console,
                               afl::sys::Log& mainLog)
    : m_gameSender(gameSender),
      m_receiver(self, *this),
      m_console(console),
      m_mainLog(mainLog),
      m_history(SCREEN_HISTORY_SIZE),
      m_blocker(root, tx("Working...")),
      m_root(root),
      m_translator(tx),
      m_waitIdCounter(3000)
{
    // TODO: remove the 'RequestDispatcher' parameter; right now, this is required by test (WidgetVerifier::run)
    // Create the ScriptSide
    class Task : public util::Request<game::Session> {
     public:
        Task(util::RequestSender<UserSide> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& session)
            {
                ScriptSide* ss = session.extra().get(SCRIPTSIDE_ID);
                if (!ss) {
                    ss = session.extra().setNew(SCRIPTSIDE_ID, new ScriptSide(m_reply));
                    ss->init(session);
                }
            }
     private:
        util::RequestSender<UserSide> m_reply;
    };
    m_gameSender.postNewRequest(new Task(m_receiver.getSender()));

    // Place the Blocker
    m_blocker.setExtent(gfx::Rectangle(gfx::Point(), m_blocker.getLayoutInfo().getPreferredSize()));
    m_root.moveWidgetToEdge(m_blocker, gfx::CenterAlign, gfx::BottomAlign, 10);
}

// Destructor.
client::si::UserSide::~UserSide()
{
    // Remove the ScriptSide
    class Task : public util::Request<game::Session> {
     public:
        virtual void handle(game::Session& session)
            {
                if (ScriptSide* ss = session.extra().get(SCRIPTSIDE_ID)) {
                    ss->done(session);
                    session.extra().setNew(SCRIPTSIDE_ID, (ScriptSide*)0);
                }
            }
    };
    m_gameSender.postNewRequest(new Task());
}

// Reset UI state.
void
client::si::UserSide::reset()
{
    m_history.clear();

    // FIXME: here?
    class Task : public util::Request<game::Session> {
     public:
        void handle(game::Session& session)
            { session.authCache().clear(); }
    };
    m_gameSender.postNewRequest(new Task());
}

/*
 *  Requests to Script Side
 */

// Post a request to execute on the ScriptSide (low-level version).
void
client::si::UserSide::postNewRequest(ScriptRequest* request)
{
    class Task : public util::Request<game::Session> {
     public:
        Task(std::auto_ptr<ScriptRequest>& p)
            : m_p(p)
            { }
        virtual void handle(game::Session& session)
            {
                if (ScriptSide* ss = session.extra().get(SCRIPTSIDE_ID)) {
                    if (m_p.get() != 0) {
                        m_p->handle(session, *ss);
                    }
                }
            }
     private:
        std::auto_ptr<ScriptRequest> m_p;
    };
    std::auto_ptr<ScriptRequest> p(request);
    m_gameSender.postNewRequest(new Task(p));
}


/*
 *  Process Functions
 */

// Continue a process after UI callout.
void
client::si::UserSide::continueProcess(RequestLink2 link)
{
    class Task : public ScriptRequest {
     public:
        Task(RequestLink2 link)
            : m_link(link)
            { }
        void handle(game::Session& session, ScriptSide& t)
            { t.continueProcess(session, m_link); }
     private:
        RequestLink2 m_link;
    };
    postNewRequest(new Task(link));
}

// Join processes into a process group.
void
client::si::UserSide::joinProcess(RequestLink2 link, RequestLink2 other)
{
    class Task : public ScriptRequest {
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
        postNewRequest(new Task(link, other));
    }
}

// Join process group.
void
client::si::UserSide::joinProcessGroup(RequestLink2 link, uint32_t oldGroup)
{
    class Task : public ScriptRequest {
     public:
        Task(RequestLink2 link, uint32_t oldGroup)
            : m_link(link),
              m_oldGroup(oldGroup)
            { }
        void handle(game::Session& session, ScriptSide& t)
            { t.joinProcessGroup(session, m_link, m_oldGroup); }
     private:
        RequestLink2 m_link;
        uint32_t m_oldGroup;
    };
    postNewRequest(new Task(link, oldGroup));
}

// Continue a process after UI callout with error.
void
client::si::UserSide::continueProcessWithFailure(RequestLink2 link, String_t error)
{
    class Task : public ScriptRequest {
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
    postNewRequest(new Task(link, error));
}

// Detach from process after UI callout.
void
client::si::UserSide::detachProcess(RequestLink2 link)
{
    class Task : public ScriptRequest {
     public:
        Task(RequestLink2 link)
            : m_link(link)
            { }
        void handle(game::Session& session, ScriptSide& t)
            { t.detachProcess(session, m_link); }
     private:
        RequestLink2 m_link;
    };
    postNewRequest(new Task(link));
}

// Set variable in process.
void
client::si::UserSide::setVariable(RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value)
{
    class Task : public ScriptRequest {
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
    postNewRequest(new Task(link, name, value));
}

/*
 *  Process Group / Wait Functions
 */

// Allocate a wait Id.
uint32_t
client::si::UserSide::allocateWaitId()
{
    return ++m_waitIdCounter;
}

// Continue a detached process.
void
client::si::UserSide::continueProcessWait(uint32_t waitId, RequestLink2 link)
{
    class ContinueTask : public ScriptRequest {
     public:
        ContinueTask(uint32_t waitId, RequestLink2 link)
            : m_waitId(waitId),
              m_link(link)
            { }
        void handle(game::Session& s, ScriptSide& t)
            { t.continueProcessWait(m_waitId, s, m_link); }
     private:
        uint32_t m_waitId;
        RequestLink2 m_link;
    };
    postNewRequest(new ContinueTask(waitId, link));
}

// Execute a task.
void
client::si::UserSide::executeTaskWait(uint32_t id, std::auto_ptr<ScriptTask> task)
{
    class StartTask : public ScriptRequest {
     public:
        StartTask(uint32_t id, std::auto_ptr<ScriptTask> task)
            : m_id(id), m_task(task)
            { }
        void handle(game::Session& s, ScriptSide& t)
            { t.executeTaskWait(m_id, m_task, s); }
     private:
        uint32_t m_id;
        std::auto_ptr<ScriptTask> m_task;
    };
    postNewRequest(new StartTask(id, task));
}

// Create ContextProvider.
client::si::ContextProvider*
client::si::UserSide::createContextProvider()
{
    if (m_controls.empty()) {
        return 0;
    } else {
        return m_controls.back()->createContextProvider();
    }
}

/*
 *  Listener Functions
 */

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

// Get current (=topmost) control.
client::si::Control*
client::si::UserSide::getControl()
{
    return m_controls.empty() ? 0 : m_controls.back();
}

// Handle successful wait.
void
client::si::UserSide::onTaskComplete(uint32_t id)
{
    for (size_t i = m_controls.size(); i > 0; --i) {
        m_controls[i-1]->onTaskComplete(id);
    }
}

/*
 *  Script-side Actions
 */

// Process an interaction.
void
client::si::UserSide::processInteraction(util::Request<UserSide>& req)
{
    // Because UI is single-threaded, and all setWaiting(WHAT), setWaiting(prev) calls happen in the same stack frame,
    // there's no risk of on setWaiting(prev) reverting the wrong call or getting lost.
    const bool prev = setWaiting(false);
    try {
        req.handle(*this);
    }
    catch (...) {
        setWaiting(prev);
        throw;
    }
    setWaiting(prev);
}

// Process a synchronous script call.
void
client::si::UserSide::processCall(UserCall& t)
{
    if (!m_controls.empty()) {
        t.handle(*m_controls.back());
    }
}


/*
 *  Wait Indicator
 */

// Set visibility of wait-indicator.
bool
client::si::UserSide::setWaiting(bool enable)
{
    bool previous = (m_blocker.getParent() != 0);
    if (enable) {
        if (m_blocker.getPreviousSibling() != 0) {
            m_root.remove(m_blocker);
        }
        if (m_blocker.getParent() == 0) {
            m_root.add(m_blocker);
        }
    } else {
        if (m_blocker.getParent() != 0) {
            m_root.remove(m_blocker);
            m_blocker.replayEvents();
        }
    }
    return previous;
}
