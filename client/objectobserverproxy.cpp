/**
  *  \file client/objectobserverproxy.cpp
  */

#include <memory>
#include "client/objectobserverproxy.hpp"
#include "client/objectobserver.hpp"
#include "util/requestreceiver.hpp"       // required because RequestReceiver/RequestSender depend on each other

client::ObjectObserverProxy::ObjectObserverProxy(util::RequestSender<game::Session> gameSender, std::auto_ptr<ObjectCursorFactory> f)
    : m_slave(gameSender, new ObjectObserver(f))
{ }

client::ObjectObserverProxy::~ObjectObserverProxy()
{ }

void
client::ObjectObserverProxy::addNewListener(ObjectListener* pl)
{
    class Job : public util::SlaveRequest<game::Session,ObjectObserver> {
     public:
        Job(std::auto_ptr<ObjectListener> pl)
            : m_listener(pl)
            { }
        void handle(game::Session& s, ObjectObserver& oo)
            { oo.addNewListener(s, m_listener.release()); }
     private:
        std::auto_ptr<ObjectListener> m_listener;
    };
    m_slave.postNewRequest(new Job(std::auto_ptr<ObjectListener>(pl)));
}

void
client::ObjectObserverProxy::postNewRequest(util::SlaveRequest<game::Session,ObjectObserver>* p)
{
    m_slave.postNewRequest(p);
}
