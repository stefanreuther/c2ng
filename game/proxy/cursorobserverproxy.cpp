/**
  *  \file game/proxy/cursorobserverproxy.cpp
  *  \brief Class game::proxy::CursorObserverProxy
  */

#include "game/proxy/cursorobserverproxy.hpp"
#include "game/map/objectobserver.hpp"
#include "game/proxy/objectlistener.hpp"

class game::proxy::CursorObserverProxy::Slave : public util::SlaveObject<Session> {
 public:
    Slave(std::auto_ptr<game::map::ObjectCursorFactory> f)
        : m_factory(f),
          m_observer(),
          m_pSession(),
          conn_objectChange(),
          m_listeners()
        { }

    ~Slave()
        { }

    virtual void init(Session& s)
        {
            if (game::map::ObjectCursor* c = m_factory->getCursor(s)) {
                m_observer.reset(new game::map::ObjectObserver(*c));
                conn_objectChange = m_observer->sig_objectChange.add(this, &Slave::onObjectChange);
                m_pSession = &s; // FIXME
            }
        }
    virtual void done(Session& /*s*/)
        {
            m_observer.reset();
        }

    void addNewListener(Session& s, ObjectListener* pl)
        {
            m_listeners.pushBackNew(pl);
            if (m_observer.get() != 0) {
                game::map::Object* p = m_observer->getCurrentObject();
                pl->handle(s, p);
            }
        }

 private:
    void onObjectChange()
        {
            if (m_observer.get() != 0) {
                game::map::Object* p = m_observer->getCurrentObject();
                for (size_t i = 0; i < m_listeners.size(); ++i) {
                    m_listeners[i]->handle(*m_pSession, p);
                }
            }
        }

    std::auto_ptr<game::map::ObjectCursorFactory> m_factory;
    std::auto_ptr<game::map::ObjectObserver> m_observer;
    Session* m_pSession;
    afl::base::SignalConnection conn_objectChange;
    afl::container::PtrVector<ObjectListener> m_listeners;
};


game::proxy::CursorObserverProxy::CursorObserverProxy(util::RequestSender<Session> gameSender, std::auto_ptr<game::map::ObjectCursorFactory> f)
    : m_slave(gameSender, new Slave(f))
{ }

game::proxy::CursorObserverProxy::~CursorObserverProxy()
{ }

void
game::proxy::CursorObserverProxy::addNewListener(ObjectListener* pl)
{
    class Job : public util::SlaveRequest<Session,Slave> {
     public:
        Job(std::auto_ptr<ObjectListener> pl)
            : m_listener(pl)
            { }
        void handle(Session& s, Slave& oo)
            { oo.addNewListener(s, m_listener.release()); }
     private:
        std::auto_ptr<ObjectListener> m_listener;
    };
    m_slave.postNewRequest(new Job(std::auto_ptr<ObjectListener>(pl)));
}
