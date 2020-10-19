/**
  *  \file game/proxy/referenceobserverproxy.cpp
  *  \brief Class game::proxy::ReferenceObserverProxy
  */

#include "game/proxy/referenceobserverproxy.hpp"
#include "game/proxy/objectlistener.hpp"
#include "afl/base/signalconnection.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/map/universe.hpp"
#include "afl/container/ptrvector.hpp"

using util::SlaveObject;
using util::SlaveRequest;

namespace {
    game::map::Object* getObject(game::Session* pSession, game::Reference ref)
    {
        if (pSession == 0) {
            return 0;
        }

        game::Game* pGame = pSession->getGame().get();
        if (pGame == 0) {
            return 0;
        }

        game::Turn* pTurn = pGame->getViewpointTurn().get();
        if (pTurn == 0) {
            return 0;
        }

        return pTurn->universe().getObject(ref);
    }
}

class game::proxy::ReferenceObserverProxy::Slave : public SlaveObject<Session> {
 public:
    Slave()
        : m_pSession(0),
          m_pObject(0),
          m_ref()
        { }
    virtual void init(Session& /*session*/)
        { }

    virtual void done(Session& /*session*/)
        {
            m_pSession = 0;
            m_pObject = 0;
            conn_viewpointTurnChange.disconnect();
            conn_objectChange.disconnect();
        }

    void addNewListener(Session& /*session*/, ObjectListener* pListener)
        {
            m_listeners.pushBackNew(pListener);
            if (m_pSession != 0 && m_pObject != 0) {
                pListener->handle(*m_pSession, m_pObject);
            }
        }

    void removeAllListeners()
        {
            m_listeners.clear();
        }

    void setReference(Session& session, Reference ref)
        {
            if (&session != m_pSession || ref != m_ref) {
                m_pSession = &session;
                m_ref = ref;

                // Attach viewpoint
                conn_viewpointTurnChange.disconnect();
                Game* pGame = session.getGame().get();
                if (pGame != 0) {
                    conn_viewpointTurnChange = pGame->sig_viewpointTurnChange.add(this, &Slave::onViewpointTurnChange);
                }

                onViewpointTurnChange();
            }
        }

    void onViewpointTurnChange()
        {
            game::map::Object* obj = getObject(m_pSession, m_ref);
            if (obj != m_pObject) {
                m_pObject = obj;
                conn_objectChange.disconnect();
                if (obj != 0) {
                    conn_objectChange = obj->sig_change.add(this, &Slave::onObjectChange);
                }
                onObjectChange();
            }
        }

    void onObjectChange()
        {
            if (m_pSession != 0) {
                for (size_t i = 0, n = m_listeners.size(); i < n; ++i) {
                    if (m_listeners[i] != 0) {
                        m_listeners[i]->handle(*m_pSession, m_pObject);
                    }
                }
            }
        }

 private:
    Session* m_pSession;
    game::map::Object* m_pObject;
    Reference m_ref;

    afl::base::SignalConnection conn_viewpointTurnChange;
    afl::base::SignalConnection conn_objectChange;

    afl::container::PtrVector<ObjectListener> m_listeners;
};


game::proxy::ReferenceObserverProxy::ReferenceObserverProxy(util::RequestSender<Session> gameSender)
    : m_slave(gameSender, new Slave())
{ }

game::proxy::ReferenceObserverProxy::~ReferenceObserverProxy()
{ }

void
game::proxy::ReferenceObserverProxy::setReference(Reference ref)
{
    class Job : public SlaveRequest<Session, Slave> {
     public:
        Job(Reference ref)
            : m_ref(ref)
            { }
        void handle(Session& s, Slave& oo)
            { oo.setReference(s, m_ref); }
     private:
        Reference m_ref;
    };
    m_slave.postNewRequest(new Job(ref));
}

void
game::proxy::ReferenceObserverProxy::addNewListener(ObjectListener* pListener)
{
    class Job : public SlaveRequest<Session, Slave> {
     public:
        Job(std::auto_ptr<ObjectListener> pl)
            : m_listener(pl)
            { }
        void handle(Session& s, Slave& oo)
            { oo.addNewListener(s, m_listener.release()); }
     private:
        std::auto_ptr<ObjectListener> m_listener;
    };
    m_slave.postNewRequest(new Job(std::auto_ptr<ObjectListener>(pListener)));
}

void
game::proxy::ReferenceObserverProxy::removeAllListeners()
{
    // FIXME: this is a stop-gap measure to get rid of temporary observers, as are used on the starchart
    // The real solution would give ObjectListeners a way to remove themselves.
    class Job : public util::SlaveRequest<Session,Slave> {
     public:
        void handle(Session& /*s*/, Slave& oo)
            { oo.removeAllListeners(); }
    };
    m_slave.postNewRequest(new Job());
}
