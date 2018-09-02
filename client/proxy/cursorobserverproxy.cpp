/**
  *  \file client/proxy/cursorobserverproxy.cpp
  */

#include "client/proxy/cursorobserverproxy.hpp"
#include "game/map/objectobserver.hpp"
#include "client/proxy/objectlistener.hpp"

class client::proxy::CursorObserverProxy::Slave : public util::SlaveObject<game::Session> {
 public:
    Slave(std::auto_ptr<ObjectCursorFactory> f)
        : m_factory(f),
          m_observer(),
          m_pSession(),
          conn_objectChange(),
          m_listeners()
        { }

    ~Slave()
        { }

    virtual void init(game::Session& s)
        {
            if (game::map::ObjectCursor* c = m_factory->getCursor(s)) {
                m_observer.reset(new game::map::ObjectObserver(*c));
                conn_objectChange = m_observer->sig_objectChange.add(this, &Slave::onObjectChange);
                m_pSession = &s; // FIXME
            }
        }
    virtual void done(game::Session& /*s*/)
        {
            m_observer.reset();
        }

    void addNewListener(game::Session& s, ObjectListener* pl)
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

    std::auto_ptr<ObjectCursorFactory> m_factory;
    std::auto_ptr<game::map::ObjectObserver> m_observer;
    game::Session* m_pSession;
    afl::base::SignalConnection conn_objectChange;
    afl::container::PtrVector<ObjectListener> m_listeners;
};


client::proxy::CursorObserverProxy::CursorObserverProxy(util::RequestSender<game::Session> gameSender, std::auto_ptr<ObjectCursorFactory> f)
    : m_slave(gameSender, new Slave(f))
{ }

client::proxy::CursorObserverProxy::~CursorObserverProxy()
{
}

void
client::proxy::CursorObserverProxy::addNewListener(ObjectListener* pl)
{
    class Job : public util::SlaveRequest<game::Session,Slave> {
     public:
        Job(std::auto_ptr<ObjectListener> pl)
            : m_listener(pl)
            { }
        void handle(game::Session& s, Slave& oo)
            { oo.addNewListener(s, m_listener.release()); }
     private:
        std::auto_ptr<ObjectListener> m_listener;
    };
    m_slave.postNewRequest(new Job(std::auto_ptr<ObjectListener>(pl)));
}
