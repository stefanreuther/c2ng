/**
  *  \file client/proxy/referenceobserverproxy.cpp
  */

#include "client/proxy/referenceobserverproxy.hpp"
#include "client/proxy/objectlistener.hpp"
#include "afl/base/signalconnection.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/map/universe.hpp"
#include "afl/container/ptrvector.hpp"

using util::SlaveObject;
using util::SlaveRequest;
using game::Reference;

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

class client::proxy::ReferenceObserverProxy::Slave : public SlaveObject<game::Session> {
 public:
    Slave()
        : m_pSession(0),
          m_pObject(0),
          m_ref()
        { }
    virtual void init(game::Session& /*session*/)
        { }

    virtual void done(game::Session& /*session*/)
        {
            m_pSession = 0;
            m_pObject = 0;
            conn_viewpointTurnChange.disconnect();
            conn_objectChange.disconnect();
        }

    void addNewListener(game::Session& /*session*/, ObjectListener* pListener)
        {
            m_listeners.pushBackNew(pListener);
        }

    void setReference(game::Session& session, Reference ref)
        {
            if (&session != m_pSession || ref != m_ref) {
                m_pSession = &session;
                m_ref = ref;

                // Attach viewpoint
                conn_viewpointTurnChange.disconnect();
                game::Game* pGame = session.getGame().get();
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
    game::Session* m_pSession;
    game::map::Object* m_pObject;
    Reference m_ref;

    afl::base::SignalConnection conn_viewpointTurnChange;
    afl::base::SignalConnection conn_objectChange;

    afl::container::PtrVector<ObjectListener> m_listeners;
};


client::proxy::ReferenceObserverProxy::ReferenceObserverProxy(util::RequestSender<game::Session> gameSender)
    : m_slave(gameSender, new Slave())
{ }

client::proxy::ReferenceObserverProxy::~ReferenceObserverProxy()
{ }

void
client::proxy::ReferenceObserverProxy::setReference(game::Reference ref)
{
    class Job : public SlaveRequest<game::Session, Slave> {
     public:
        Job(Reference ref)
            : m_ref(ref)
            { }
        void handle(game::Session& s, Slave& oo)
            { oo.setReference(s, m_ref); }
     private:
        Reference m_ref;
    };
    m_slave.postNewRequest(new Job(ref));
}

void
client::proxy::ReferenceObserverProxy::addNewListener(ObjectListener* pListener)
{
    class Job : public SlaveRequest<game::Session, Slave> {
     public:
        Job(std::auto_ptr<ObjectListener> pl)
            : m_listener(pl)
            { }
        void handle(game::Session& s, Slave& oo)
            { oo.addNewListener(s, m_listener.release()); }
     private:
        std::auto_ptr<ObjectListener> m_listener;
    };
    m_slave.postNewRequest(new Job(std::auto_ptr<ObjectListener>(pListener)));
}
