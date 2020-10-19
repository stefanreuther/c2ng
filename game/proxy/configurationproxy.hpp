/**
  *  \file game/proxy/configurationproxy.hpp
  *  \brief Class game::proxy::ConfigurationProxy
  */
#ifndef C2NG_GAME_PROXY_CONFIGURATIONPROXY_HPP
#define C2NG_GAME_PROXY_CONFIGURATIONPROXY_HPP

#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/numberformatter.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Proxy to access configuration items.
        (As of 20200808, incomplete.) */
    class ConfigurationProxy {
     public:
        /** Constructor.
            \param gameSender Game sender */
        ConfigurationProxy(util::RequestSender<Session> gameSender);

        /** Get number formatter.
            Obtain a formatter to format numbers and population counts according to user's choice.
            \param link Synchronisation */
        util::NumberFormatter getNumberFormatter(WaitIndicator& link);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
