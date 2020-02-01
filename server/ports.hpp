/**
  *  \file server/ports.hpp
  *  \brief Default Port Numbers
  *
  *  Port numbers are normally defined in c2config.txt; see ConfigurationHandler.
  *  Production port numbers therefore differ from these.
  *  Likewise, testing (c2systest) allocates port numbers from a different range.
  *
  *  This file defines the defaults, so all servers agree upon defaults if a configuration element is missing.
  */
#ifndef C2NG_SERVER_PORTS_HPP
#define C2NG_SERVER_PORTS_HPP

#include "afl/base/types.hpp"

namespace server {

    // Web client / user file
    const uint16_t ROUTER_PORT   = 9999;
    const uint16_t FILE_PORT     = 9998;

    // Host
    const uint16_t HOST_PORT     = 7777;
    const uint16_t HOSTFILE_PORT = 7776;

    // Others
    const uint16_t MAILOUT_PORT  = 21212;
    const uint16_t FORMAT_PORT   = 6665;
    const uint16_t TALK_PORT     = 5555;
    const uint16_t DB_PORT       = 6379;
    const uint16_t NNTP_PORT     = 1119;
    const uint16_t USER_PORT     = 6526;

    const uint16_t SMTP_PORT     = 25;

    // Web
    const uint16_t WWW_PORT      = 80;
    const uint16_t MONITOR_PORT  = 8080;
    const uint16_t PROXY_PORT    = 1080;

    // Default IP to bind to
    const char*const DEFAULT_ADDRESS = "127.0.0.1";

}

#endif
