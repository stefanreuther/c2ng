/**
  *  \file server/talk/configuration.cpp
  *  \brief Structure server::talk::Configuration
  */

#include "server/talk/configuration.hpp"

// Constructor.
server::talk::Configuration::Configuration()
    : messageIdSuffix("@localhost"),
      baseUrl("http://localhost/"),
      userKey("unset"),
      pathHost("localhost")
{ }
