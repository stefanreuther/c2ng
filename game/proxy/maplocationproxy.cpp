/**
  *  \file game/proxy/maplocationproxy.cpp
  *  \brief Class game::proxy::MapLocationProxy
  */

#include "game/proxy/maplocationproxy.hpp"
#include "game/game.hpp"
#include "game/map/cursors.hpp"
#include "game/map/location.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/turn.hpp"

using game::map::Configuration;
using game::map::Point;

namespace {
    void initLocalMapConfig(const game::Root& root, Configuration& config)
    {
        config.initFromConfiguration(root.hostConfiguration(), root.userConfiguration());
    }
}

class game::proxy::MapLocationProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<MapLocationProxy> reply)
        : m_reply(reply),
          m_session(session),
          m_inhibitPositionChange(false),
          m_localConfig(),
          conn_positionChange()
        {
            if (Game* pGame = session.getGame().get()) {
                conn_positionChange = pGame->cursors().location().sig_positionChange.add(this, &Trampoline::onPositionChange);
            }
            if (Root* pRoot = session.getRoot().get()) {
                conn_prefChange = pRoot->userConfiguration().sig_change.add(this, &Trampoline::onConfigChange);
                conn_configChange = pRoot->hostConfiguration().sig_change.add(this, &Trampoline::onConfigChange);
                initLocalMapConfig(*pRoot, m_localConfig);
            }
        }

    void onPositionChange(Point pt)
        {
            if (!m_inhibitPositionChange) {
                sendPositionChange(pt);
            }
        }

    void onConfigChange()
        {
            if (Root* pRoot = m_session.getRoot().get()) {
                Configuration tmpConfig;
                initLocalMapConfig(*pRoot, tmpConfig);
                if (tmpConfig != m_localConfig) {
                    m_localConfig = tmpConfig;
                    sendConfigChange();
                }
            }
        }

    void sendPositionChange(Point pt)
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
            if (Game* pGame = m_session.getGame().get()) {
                game::map::Location& loc = pGame->cursors().location();
                loc.getPosition().get(pt);
                ref = loc.getReference();
            }
            m_reply.postNewRequest(new Response(ref, pt, m_localConfig));
        }

    void sendConfigChange()
        {
            class Response : public util::Request<MapLocationProxy> {
             public:
                Response(const Configuration& config)
                    : m_config(config)
                    { }
                virtual void handle(MapLocationProxy& proxy)
                    { proxy.sig_configChange.raise(m_config); }
             private:
                const Configuration m_config;
            };
            m_reply.postNewRequest(new Response(m_localConfig));
        }

    template<typename T>
    void setPosition(T t)
        {
            if (Game* pGame = m_session.getGame().get()) {
                m_inhibitPositionChange = true;
                pGame->cursors().location().set(t);
                m_inhibitPositionChange = false;

                const Point pt = pGame->cursors().location().getPosition().orElse(Point());
                sendPositionChange(pt);
            }
        }

    void browse(game::map::Location::BrowseFlags_t flags)
        {
            if (Game* pGame = m_session.getGame().get()) {
                m_inhibitPositionChange = true;
                pGame->cursors().location().browse(flags);
                m_inhibitPositionChange = false;

                const Point pt = pGame->cursors().location().getPosition().orElse(Point());
                sendPositionChange(pt);
                m_reply.postRequest(&MapLocationProxy::emitBrowseResult, pGame->cursors().location().getEffectiveReference(), pt);
            }
        }

    bool getOtherPosition(Id_t shipId, Point& result)
        {
            if (Game* pGame = m_session.getGame().get()) {
                return pGame->cursors().location().getOtherPosition(shipId).get(result);
            } else {
                return false;
            }
        }

 private:
    /* Links */
    util::RequestSender<MapLocationProxy> m_reply;
    Session& m_session;

    /* Inhibit implicit position changes to avoid multiple/overlapping reports */
    bool m_inhibitPositionChange;

    /* Local copy of the configuration.
       We need to maintain our own copy because the global copy is updated by Session from the same callbacks we use,
       and we cannot know whether Session has already updated it when we see it. */
    Configuration m_localConfig;

    /* Signal connections */
    afl::base::SignalConnection conn_positionChange;
    afl::base::SignalConnection conn_prefChange;
    afl::base::SignalConnection conn_configChange;
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
    m_trampoline.postRequest(&Trampoline::setPosition<Point>, pt);
}

void
game::proxy::MapLocationProxy::browse(game::map::Location::BrowseFlags_t flags)
{
    ++m_numOutstandingRequests;
    m_trampoline.postRequest(&Trampoline::browse, flags);
}

bool
game::proxy::MapLocationProxy::getOtherPosition(WaitIndicator& ind, game::Id_t shipId, game::map::Point& result)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Id_t shipId)
            : m_shipId(shipId), m_ok(false), m_result()
            { }
        virtual void handle(Trampoline& tpl)
            { m_ok = tpl.getOtherPosition(m_shipId, m_result); }
        bool isOK() const
            { return m_ok; }
        const Point& getResult() const
            { return m_result; }
     private:
        const Id_t m_shipId;
        bool m_ok;
        Point m_result;
    };

    Task t(shipId);
    ind.call(m_trampoline, t);
    if (t.isOK()) {
        result = t.getResult();
        return true;
    } else {
        return false;
    }
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
