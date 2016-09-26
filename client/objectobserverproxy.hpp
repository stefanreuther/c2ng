/**
  *  \file client/objectobserverproxy.hpp
  */
#ifndef C2NG_CLIENT_OBJECTOBSERVERPROXY_HPP
#define C2NG_CLIENT_OBJECTOBSERVERPROXY_HPP

#include <memory>
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "util/slaverequestsender.hpp"
#include "util/slaverequest.hpp"

namespace client {

    class ObjectObserver;
    class ObjectListener;
    class ObjectCursorFactory;

    class ObjectObserverProxy {
     public:
        ObjectObserverProxy(util::RequestSender<game::Session> gameSender, std::auto_ptr<ObjectCursorFactory> f);
        ~ObjectObserverProxy();

        void addNewListener(ObjectListener* pl);
        void postNewRequest(util::SlaveRequest<game::Session,ObjectObserver>* p);

     private:
        util::SlaveRequestSender<game::Session, ObjectObserver> m_slave;
    };

}

#endif
