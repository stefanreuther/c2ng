/**
  *  \file server/router/configuration.hpp
  */
#ifndef C2NG_SERVER_ROUTER_CONFIGURATION_HPP
#define C2NG_SERVER_ROUTER_CONFIGURATION_HPP

#include "afl/string/string.hpp"

namespace server { namespace router {

    struct Configuration {
        Configuration();

        String_t serverPath;               // ex arg_server
        int32_t normalTimeout;             // ex arg_timeout
        int32_t virginTimeout;             // ex arg_virgintimeout
        size_t maxSessions;                // ex arg_sessionlimit
        bool newSessionsWin;               // ex arg_newsessionswin
        bool enableFileNotify;             // ex arg_filenotify
    };

} }

#endif
