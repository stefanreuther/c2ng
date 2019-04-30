/**
  *  \file server/user/configuration.cpp
  *  \brief Structure server::user::Configuration
  */

#include "server/user/configuration.hpp"

// Constructor.
server::user::Configuration::Configuration()
    : userKey("unset"),
      userDataMaxKeySize(1024),
      userDataMaxValueSize(1024*1024),
      userDataMaxTotalSize(1536*1024)
{ }
