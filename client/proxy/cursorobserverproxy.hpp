/**
  *  \file client/proxy/cursorobserverproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_CURSOROBSERVERPROXY_HPP
#define C2NG_CLIENT_PROXY_CURSOROBSERVERPROXY_HPP

#include <memory>
#include "client/proxy/objectobserver.hpp"
#include "game/session.hpp"
#include "util/requestreceiver.hpp"
#include "util/slaverequestsender.hpp"
#include "client/objectcursorfactory.hpp"

namespace client { namespace proxy {

    class CursorObserverProxy : public ObjectObserver {
     public:
        CursorObserverProxy(util::RequestSender<game::Session> gameSender, std::auto_ptr<ObjectCursorFactory> f);
        ~CursorObserverProxy();

        virtual void addNewListener(ObjectListener* pl);

     private:
        class Slave;

        util::SlaveRequestSender<game::Session, Slave> m_slave;
    };

} }

#endif
