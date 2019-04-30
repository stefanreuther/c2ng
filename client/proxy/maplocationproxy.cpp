/**
  *  \file client/proxy/maplocationproxy.cpp
  *  \brief Class client::proxy::MapLocationProxy
  */

#include "client/proxy/maplocationproxy.hpp"
#include "game/game.hpp"
#include "game/map/cursors.hpp"
#include "game/map/location.hpp"
#include "game/turn.hpp"

using game::Reference;
using game::Session;
using game::map::Configuration;
using game::map::Point;

namespace {
    /* Common code for both setPosition() signatures */
    template<typename T>
    class SetQuery : public util::Request<Session> {
     public:
        SetQuery(const T& t)
            : m_t(t)
            { }
        virtual void handle(Session& s)
            {
                if (game::Game* pGame = s.getGame().get()) {
                    pGame->cursors().location().set(m_t);
                }
            }
     private:
        T m_t;
    };
}

class client::proxy::MapLocationProxy::Trampoline : public util::SlaveObject<game::Session> {
 public:
    Trampoline(util::RequestSender<MapLocationProxy> reply)
        : m_reply(reply),
          conn_positionChange()
        { }
    virtual void init(game::Session& session)
        {
            if (game::Game* pGame = session.getGame().get()) {
                conn_positionChange = pGame->cursors().location().sig_positionChange.add(this, &Trampoline::onPositionChange);
            }
        }
    virtual void done(game::Session& /*session*/)
        {
            conn_positionChange.disconnect();
        }

    void onPositionChange(game::map::Point pt)
        {
            class Job : public util::Request<MapLocationProxy> {
             public:
                Job(game::map::Point pt)
                    : m_point(pt)
                    { }
                void handle(MapLocationProxy& p)
                    { p.sig_positionChange.raise(m_point); }
             private:
                game::map::Point m_point;
            };
            m_reply.postNewRequest(new Job(pt));
        }

 private:
    util::RequestSender<MapLocationProxy> m_reply;
    afl::base::SignalConnection conn_positionChange;
};


// Constructor.
client::proxy::MapLocationProxy::MapLocationProxy(util::RequestDispatcher& reply, util::RequestSender<game::Session> gameSender)
    : sig_locationResult(),
      sig_positionChange(),
      m_gameSender(gameSender),
      m_reply(reply, *this),
      m_trampoline(gameSender, new Trampoline(m_reply.getSender()))
{ }

// Destructor.
client::proxy::MapLocationProxy::~MapLocationProxy()
{ }

// Post a request to query the current location.
void
client::proxy::MapLocationProxy::postQueryLocation()
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

    // Query from UI to game thread
    class Query : public util::Request<Session> {
     public:
        Query(util::RequestSender<MapLocationProxy> reply)
            : m_reply(reply)
            { }
        void handle(Session& session)
            {
                Reference ref;
                Point pt(2000, 2000);
                Configuration config;
                if (game::Game* pGame = session.getGame().get()) {
                    game::map::Location& loc = pGame->cursors().location();
                    loc.getPosition(pt);
                    ref = loc.getReference();
                    config = pGame->currentTurn().universe().config();
                }
                m_reply.postNewRequest(new Response(ref, pt, config));
            }
     private:
        util::RequestSender<MapLocationProxy> m_reply;
    };

    m_gameSender.postNewRequest(new Query(m_reply.getSender()));
}

// Set location to point.
void
client::proxy::MapLocationProxy::setPosition(game::map::Point pt)
{
    m_gameSender.postNewRequest(new SetQuery<Point>(pt));
}

// Set location to reference.
void
client::proxy::MapLocationProxy::setPosition(game::Reference ref)
{
    m_gameSender.postNewRequest(new SetQuery<Reference>(ref));
}
