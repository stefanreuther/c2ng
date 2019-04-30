/**
  *  \file client/proxy/configurationproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_CONFIGURATIONPROXY_HPP
#define C2NG_CLIENT_PROXY_CONFIGURATIONPROXY_HPP

#include "util/numberformatter.hpp"
#include "client/downlink.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace proxy {

    class ConfigurationProxy {
     public:
        ConfigurationProxy(util::RequestSender<game::Session> gameSender);

        util::NumberFormatter getNumberFormatter(Downlink& link);

     private:
        util::RequestSender<game::Session> m_gameSender;
    };

} }

#endif
