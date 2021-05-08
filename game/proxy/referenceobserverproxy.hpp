/**
  *  \file game/proxy/referenceobserverproxy.hpp
  *  \brief Class game::proxy::ReferenceObserverProxy
  */
#ifndef C2NG_GAME_PROXY_REFERENCEOBSERVERPROXY_HPP
#define C2NG_GAME_PROXY_REFERENCEOBSERVERPROXY_HPP

#include "game/proxy/objectobserver.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "util/requestreceiver.hpp"

namespace game { namespace proxy {

    class ReferenceObserverProxy : public ObjectObserver {
     public:
        ReferenceObserverProxy(util::RequestSender<Session> gameSender);
        ~ReferenceObserverProxy();

        void setReference(Reference ref);

        virtual void addNewListener(ObjectListener* pListener);

        void removeAllListeners();

     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestSender<Trampoline> m_trampoline;
    };

} }


#endif
