/**
  *  \file game/proxy/cursorobserverproxy.cpp
  *  \brief Class game::proxy::CursorObserverProxy
  */

#include "game/proxy/cursorobserverproxy.hpp"
#include "game/map/objectobserver.hpp"
#include "game/proxy/objectlistener.hpp"

class game::proxy::CursorObserverProxy::Trampoline {
 public:
    Trampoline(Session& session, std::auto_ptr<game::map::ObjectCursorFactory> f)
        : m_factory(f),
          m_observer(),
          m_session(session),
          conn_objectChange(),
          m_listeners()
        {
            if (game::map::ObjectCursor* c = m_factory->getCursor(session)) {
                m_observer.reset(new game::map::ObjectObserver(*c));
                conn_objectChange = m_observer->sig_objectChange.add(this, &Trampoline::onObjectChange);
            }
        }

    void addNewListener(ObjectListener* pl)
        {
            m_listeners.pushBackNew(pl);
            if (m_observer.get() != 0) {
                game::map::Object* p = m_observer->getCurrentObject();
                pl->handle(m_session, p);
            }
        }

 private:
    void onObjectChange()
        {
            if (m_observer.get() != 0) {
                game::map::Object* p = m_observer->getCurrentObject();
                for (size_t i = 0; i < m_listeners.size(); ++i) {
                    m_listeners[i]->handle(m_session, p);
                }
            }
        }

    std::auto_ptr<game::map::ObjectCursorFactory> m_factory;
    std::auto_ptr<game::map::ObjectObserver> m_observer;
    Session& m_session;
    afl::base::SignalConnection conn_objectChange;
    afl::container::PtrVector<ObjectListener> m_listeners;
};



class game::proxy::CursorObserverProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(std::auto_ptr<game::map::ObjectCursorFactory>& f)
        : m_factory(f)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_factory); }
 private:
    std::auto_ptr<game::map::ObjectCursorFactory> m_factory;
};



game::proxy::CursorObserverProxy::CursorObserverProxy(util::RequestSender<Session> gameSender, std::auto_ptr<game::map::ObjectCursorFactory> f)
    : m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(f)))
{ }

game::proxy::CursorObserverProxy::~CursorObserverProxy()
{ }

void
game::proxy::CursorObserverProxy::addNewListener(ObjectListener* pl)
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
    m_trampoline.postNewRequest(new Job(std::auto_ptr<ObjectListener>(pl)));
}
