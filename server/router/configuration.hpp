/**
  *  \file server/router/configuration.hpp
  *  \brief Structure server::router::Configuration
  */
#ifndef C2NG_SERVER_ROUTER_CONFIGURATION_HPP
#define C2NG_SERVER_ROUTER_CONFIGURATION_HPP

#include "afl/string/string.hpp"

namespace server { namespace router {

    /** Configuration for Session Router service. */
    struct Configuration {
        /** Default constructor.
            Sets everything to defaults. */
        Configuration();

        /** Path to c2play-server binary (Router.Server). */
        String_t serverPath;               // ex arg_server

        /** Timeout for used sessions, in seconds (Router.Timeout). */
        int32_t normalTimeout;             // ex arg_timeout

        /** Timeout for unused sessions, in seconds (Router.VirginTimeout). */
        int32_t virginTimeout;             // ex arg_virgintimeout

        /** Maximum number of parallel sessions (Router.MaxSessions). */
        size_t maxSessions;                // ex arg_sessionlimit

        /** true if new sessions displace old ones (Router.NewSessionsWin). */
        bool newSessionsWin;               // ex arg_newsessionswin
    };

} }

#endif
