/**
  *  \file server/router/configuration.cpp
  *  \brief Structure server::router::Configuration
  */

#include "server/router/configuration.hpp"

server::router::Configuration::Configuration()
    : serverPath("./c2play-server"),
      normalTimeout(10000),
      virginTimeout(60),
      maxSessions(10),
      newSessionsWin(false)
{ }
