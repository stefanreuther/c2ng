/**
  *  \file client/si/userside.cpp
  *  \brief Class client::si::UserSide
  */

#include "client/si/userside.hpp"
#include "afl/string/format.hpp"
#include "client/si/control.hpp"
#include "client/si/scriptside.hpp"
#include "game/extraidentifier.hpp"

const size_t SCREEN_HISTORY_SIZE = 50;

const game::ExtraIdentifier<game::Session, client::si::ScriptSide> client::si::SCRIPTSIDE_ID = {{}};


/*
 *  ScriptSenderImpl - Adaptor to send to a ScriptSide
 */
class client::si::UserSide::ScriptSenderImpl : public util::RequestSender<ScriptSide>::Impl {
 public:
    ScriptSenderImpl(const util::RequestSender<game::Session>& gameSender)
        : m_gameSender(gameSender)
        { }
    virtual void postNewRequest(util::Request<client::si::ScriptSide>* req)
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
                                m_p->handle(*ss);
                            }
                        }
                    }
             private:
                std::auto_ptr<ScriptRequest> m_p;
            };
            std::auto_ptr<ScriptRequest> p(req);
            m_gameSender.postNewRequest(new Task(p));
        }
 private:
    util::RequestSender<game::Session> m_gameSender;
};



// Constructor.
client::si::UserSide::UserSide(ui::Root& root,
                               util::RequestSender<game::Session> gameSender,
                               afl::string::Translator& tx,
                               util::RequestDispatcher& self,
                               util::MessageCollector& console,
                               afl::sys::Log& mainLog)
    : m_gameSender(gameSender),
      m_scriptSender(*new ScriptSenderImpl(gameSender)),
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
                    ss = session.extra().setNew(SCRIPTSIDE_ID, new ScriptSide(m_reply, session));
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
            { session.extra().setNew(SCRIPTSIDE_ID, (ScriptSide*)0); }
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
    m_scriptSender.postNewRequest(request);
}


/*
 *  Process Functions
 */

// Continue a process after UI callout.
void
client::si::UserSide::continueProcess(RequestLink2 link)
{
    m_scriptSender.postRequest(&ScriptSide::continueProcess, link);
}

// Join processes into a process group.
void
client::si::UserSide::joinProcess(RequestLink2 link, RequestLink2 other)
{
    m_scriptSender.postRequest(&ScriptSide::joinProcess, link, other);
}

// Join process group.
void
client::si::UserSide::joinProcessGroup(RequestLink2 link, uint32_t oldGroup)
{
    m_scriptSender.postRequest(&ScriptSide::joinProcessGroup, link, oldGroup);
}

// Continue a process after UI callout with error.
void
client::si::UserSide::continueProcessWithFailure(RequestLink2 link, String_t error)
{
    m_scriptSender.postRequest(&ScriptSide::continueProcessWithFailure, link, error);
}

// Detach from process after UI callout.
void
client::si::UserSide::detachProcess(RequestLink2 link)
{
    m_scriptSender.postRequest(&ScriptSide::detachProcess, link);
}

// Set variable in process.
void
client::si::UserSide::setVariable(RequestLink2 link, String_t name, std::auto_ptr<afl::data::Value> value)
{
    m_scriptSender.postRequest(&ScriptSide::setVariable, link, name, value);
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
    m_scriptSender.postRequest(&ScriptSide::continueProcessWait, waitId, link);
}

// Execute a task.
void
client::si::UserSide::executeTaskWait(uint32_t waitId, std::auto_ptr<ScriptTask> task)
{
    m_scriptSender.postRequest(&ScriptSide::executeTaskWait, waitId, task);
}

// Create ContextProvider.
game::interface::ContextProvider*
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
client::si::UserSide::processCall(util::Request<Control>& t)
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
