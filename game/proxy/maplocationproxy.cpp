/**
  *  \file game/proxy/maplocationproxy.cpp
  *  \brief Class game::proxy::MapLocationProxy
  */

#include "game/proxy/maplocationproxy.hpp"
#include "game/game.hpp"
#include "game/map/cursors.hpp"
#include "game/map/location.hpp"
#include "game/turn.hpp"

using game::map::Configuration;
using game::map::Point;

class game::proxy::MapLocationProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<MapLocationProxy> reply)
        : m_reply(reply),
          m_session(session),
          m_inhibitPositionChange(false),
          conn_positionChange()
        {
            if (Game* pGame = session.getGame().get()) {
                conn_positionChange = pGame->cursors().location().sig_positionChange.add(this, &Trampoline::onPositionChange);
            }
        }

    void onPositionChange(game::map::Point pt)
        {
            if (!m_inhibitPositionChange) {
                sendPositionChange(pt);
            }
        }

    void sendPositionChange(game::map::Point pt)
        { m_reply.postRequest(&MapLocationProxy::emitPositionChange, pt); }

    void sendLocation()
        {
            // Response from game to UI thread
            class Response : public util::Request<MapLocationProxy> {
             public:
                Response(const Reference& ref, const Point& pt, const Configuration& config)
                    : m_reference(ref), m_point(pt), m_config(config)
                    { }
                void handle(MapLocationProxy& proxy)
                    { proxy.sig_locationResult.raise(m_reference, m_point, m_config); }
             private:
                Reference m_reference;
                Point m_point;
                Configuration m_config;
            };

            Reference ref;
            Point pt(2000, 2000);
            Configuration config;
            if (Game* pGame = m_session.getGame().get()) {
                game::map::Location& loc = pGame->cursors().location();
                loc.getPosition(pt);
                ref = loc.getReference();
                config = pGame->mapConfiguration();
            }
            m_reply.postNewRequest(new Response(ref, pt, config));
        }

    template<typename T>
    void setPosition(T t)
        {
            if (game::Game* pGame = m_session.getGame().get()) {
                m_inhibitPositionChange = true;
                pGame->cursors().location().set(t);
                m_inhibitPositionChange = false;

                game::map::Point pt;
                pGame->cursors().location().getPosition(pt);
                sendPositionChange(pt);
            }
        }

    void browse(game::map::Location::BrowseFlags_t flags)
        {
            if (game::Game* pGame = m_session.getGame().get()) {
                m_inhibitPositionChange = true;
                pGame->cursors().location().browse(flags);
                m_inhibitPositionChange = false;

                game::map::Point pt;
                pGame->cursors().location().getPosition(pt);
                sendPositionChange(pt);
                m_reply.postRequest(&MapLocationProxy::emitBrowseResult, pGame->cursors().location().getEffectiveReference(), pt);
            }
        }

 private:
    util::RequestSender<MapLocationProxy> m_reply;
    Session& m_session;
    bool m_inhibitPositionChange;
    afl::base::SignalConnection conn_positionChange;
};

class game::proxy::MapLocationProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<MapLocationProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<MapLocationProxy> m_reply;
};


// Constructor.
game::proxy::MapLocationProxy::MapLocationProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : sig_locationResult(),
      sig_positionChange(),
      m_reply(reply, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender()))),
      m_numOutstandingRequests(0)
{ }

// Destructor.
game::proxy::MapLocationProxy::~MapLocationProxy()
{ }

// Post a request to query the current location.
void
game::proxy::MapLocationProxy::postQueryLocation()
{
    m_trampoline.postRequest(&Trampoline::sendLocation);
}

// Set location to point.
void
game::proxy::MapLocationProxy::setPosition(game::map::Point pt)
{
    ++m_numOutstandingRequests;
    m_trampoline.postRequest(&Trampoline::setPosition<game::map::Point>, pt);
}

void
game::proxy::MapLocationProxy::browse(game::map::Location::BrowseFlags_t flags)
{
    ++m_numOutstandingRequests;
    m_trampoline.postRequest(&Trampoline::browse, flags);
}

// Set location to reference.
void
game::proxy::MapLocationProxy::setPosition(Reference ref)
{
    ++m_numOutstandingRequests;
    m_trampoline.postRequest(&Trampoline::setPosition<Reference>, ref);
}

void
game::proxy::MapLocationProxy::emitPositionChange(game::map::Point pt)
{
    if (m_numOutstandingRequests > 0) {
        --m_numOutstandingRequests;
    }
    if (m_numOutstandingRequests == 0) {
        sig_positionChange.raise(pt);
    }
}

void
game::proxy::MapLocationProxy::emitBrowseResult(Reference ref, game::map::Point pt)
{
    sig_browseResult.raise(ref, pt);
}
