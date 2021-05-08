/**
  *  \file game/proxy/reverterproxy.cpp
  *  \brief Class game::proxy::ReverterProxy
  */

#include <stdexcept>
#include "game/proxy/reverterproxy.hpp"
#include "game/game.hpp"
#include "game/map/reverter.hpp"
#include "game/map/universe.hpp"
#include "game/ref/sortbyid.hpp"
#include "game/turn.hpp"

class game::proxy::ReverterProxy::Trampoline {
 public:
    Trampoline(Session& session)
        : m_session(session)
        { }

    void init(game::map::Point pt, Status& status)
        {
            // Clear everything
            m_reverter.reset();
            status.modes.clear();
            status.list.clear();

            // Obtain new LocationReverter
            if (Game* g = m_session.getGame().get()) {
                if (Turn* t = g->getViewpointTurn().get()) {
                    if (game::map::Reverter* rev = t->universe().getReverter()) {
                        m_reverter.reset(rev->createLocationReverter(pt));
                    }
                }
            }

            // Build status
            if (m_reverter.get() != 0) {
                status.modes = m_reverter->getAvailableModes();
                status.list.add(m_reverter->getAffectedObjects(), m_session, game::ref::SortById(), game::ref::SortById());
            }
        }

    void commit(Modes_t modes)
        {
            if (m_reverter.get() != 0) {
                try {
                    m_reverter->commit(modes);
                }
                catch (std::exception& e) {
                    m_session.log().write(afl::sys::LogListener::Error, "game.proxy.reverter", m_session.translator()("Failed to revert location"), e);
                }
            }
        }

 private:
    Session& m_session;
    std::auto_ptr<game::map::LocationReverter> m_reverter;
};


class game::proxy::ReverterProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session); }
};




// Constructor.
game::proxy::ReverterProxy::ReverterProxy(util::RequestSender<Session> gameSender)
    : m_sender(gameSender.makeTemporary(new TrampolineFromSession()))
{ }

// Destructor.
game::proxy::ReverterProxy::~ReverterProxy()
{ }

// Initialize.
void
game::proxy::ReverterProxy::init(WaitIndicator& link, game::map::Point pt, Status& status)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(game::map::Point pt, Status& status)
            : m_point(pt), m_status(status)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.init(m_point, m_status); }
     private:
        game::map::Point m_point;
        Status& m_status;
    };

    Task t(pt, status);
    link.call(m_sender, t);
}

// Commit.
void
game::proxy::ReverterProxy::commit(Modes_t modes)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Modes_t modes)
            : m_modes(modes)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.commit(m_modes); }
     private:
        Modes_t m_modes;
    };
    m_sender.postNewRequest(new Task(modes));
}
