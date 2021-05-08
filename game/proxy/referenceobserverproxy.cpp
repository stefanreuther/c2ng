/**
  *  \file game/proxy/referenceobserverproxy.cpp
  *  \brief Class game::proxy::ReferenceObserverProxy
  */

#include "game/proxy/referenceobserverproxy.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/turn.hpp"

namespace {
    game::map::Object* getObject(game::Session& session, game::Reference ref)
    {
        game::Game* pGame = session.getGame().get();
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

class game::proxy::ReferenceObserverProxy::Trampoline {
 public:
    Trampoline(Session& session)
        : m_session(session),
          m_pObject(0),
          m_ref()
        { }

    void addNewListener(ObjectListener* pListener)
        {
            m_listeners.pushBackNew(pListener);
            if (m_pObject != 0) {
                pListener->handle(m_session, m_pObject);
            }
        }

    void removeAllListeners()
        {
            m_listeners.clear();
        }

    void setReference(Reference ref)
        {
            if (ref != m_ref) {
                m_ref = ref;

                // Attach viewpoint
                conn_viewpointTurnChange.disconnect();
                Game* pGame = m_session.getGame().get();
                if (pGame != 0) {
                    conn_viewpointTurnChange = pGame->sig_viewpointTurnChange.add(this, &Trampoline::onViewpointTurnChange);
                }

                onViewpointTurnChange();
            }
        }

    void onViewpointTurnChange()
        {
            game::map::Object* obj = getObject(m_session, m_ref);
            if (obj != m_pObject) {
                m_pObject = obj;
                conn_objectChange.disconnect();
                if (obj != 0) {
                    conn_objectChange = obj->sig_change.add(this, &Trampoline::onObjectChange);
                }
                onObjectChange();
            }
        }

    void onObjectChange()
        {
            for (size_t i = 0, n = m_listeners.size(); i < n; ++i) {
                if (m_listeners[i] != 0) {
                    m_listeners[i]->handle(m_session, m_pObject);
                }
            }
        }

 private:
    Session& m_session;
    game::map::Object* m_pObject;
    Reference m_ref;

    afl::base::SignalConnection conn_viewpointTurnChange;
    afl::base::SignalConnection conn_objectChange;

    afl::container::PtrVector<ObjectListener> m_listeners;
};


class game::proxy::ReferenceObserverProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session); }
};



game::proxy::ReferenceObserverProxy::ReferenceObserverProxy(util::RequestSender<Session> gameSender)
    : m_trampoline(gameSender.makeTemporary(new TrampolineFromSession()))
{ }

game::proxy::ReferenceObserverProxy::~ReferenceObserverProxy()
{ }

void
game::proxy::ReferenceObserverProxy::setReference(Reference ref)
{
    class Job : public util::Request<Trampoline> {
     public:
        Job(Reference ref)
            : m_ref(ref)
            { }
        void handle(Trampoline& oo)
            { oo.setReference(m_ref); }
     private:
        Reference m_ref;
    };
    m_trampoline.postNewRequest(new Job(ref));
}

void
game::proxy::ReferenceObserverProxy::addNewListener(ObjectListener* pListener)
{
    class Job : public util::Request<Trampoline> {
     public:
        Job(std::auto_ptr<ObjectListener> pl)
            : m_listener(pl)
            { }
        void handle(Trampoline& oo)
            { oo.addNewListener(m_listener.release()); }
     private:
        std::auto_ptr<ObjectListener> m_listener;
    };
    m_trampoline.postNewRequest(new Job(std::auto_ptr<ObjectListener>(pListener)));
}

void
game::proxy::ReferenceObserverProxy::removeAllListeners()
{
    // FIXME: this is a stop-gap measure to get rid of temporary observers, as are used on the starchart
    // The real solution would give ObjectListeners a way to remove themselves.
    class Job : public util::Request<Trampoline> {
     public:
        void handle(Trampoline& oo)
            { oo.removeAllListeners(); }
    };
    m_trampoline.postNewRequest(new Job());
}
