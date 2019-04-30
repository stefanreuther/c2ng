/**
  *  \file client/proxy/referenceobserverproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_REFERENCEOBSERVERPROXY_HPP
#define C2NG_CLIENT_PROXY_REFERENCEOBSERVERPROXY_HPP

#include "client/proxy/objectobserver.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "util/requestreceiver.hpp"
#include "util/slaverequestsender.hpp"

namespace client { namespace proxy {

    class ReferenceObserverProxy : public ObjectObserver {
     public:
        ReferenceObserverProxy(util::RequestSender<game::Session> gameSender);
        ~ReferenceObserverProxy();

        void setReference(game::Reference ref);

        virtual void addNewListener(ObjectListener* pListener);

        void removeAllListeners();

     private:
        class Slave;

        util::SlaveRequestSender<game::Session, Slave> m_slave;
    };

} }

#endif
