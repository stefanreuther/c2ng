/**
  *  \file client/proxy/friendlycodeproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_FRIENDLYCODEPROXY_HPP
#define C2NG_CLIENT_PROXY_FRIENDLYCODEPROXY_HPP

#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "client/downlink.hpp"

namespace client { namespace proxy {

    class FriendlyCodeProxy {
     public:
        FriendlyCodeProxy(util::RequestSender<game::Session> gameSender);

        String_t generateRandomCode(Downlink& link);

     private:
        util::RequestSender<game::Session> m_gameSender;
    };

} }

#endif
