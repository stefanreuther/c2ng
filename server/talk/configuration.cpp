/**
  *  \file server/talk/configuration.cpp
  *  \brief Structure server::talk::Configuration
  */

#include "server/talk/configuration.hpp"

// Constructor.
server::talk::Configuration::Configuration()
    : messageIdSuffix("@localhost"),
      baseUrl("http://localhost/"),
      pathHost("localhost"),
      rateMinimum(-100),
      rateMaximum(50),
      rateCooldown(50),
      rateInterval(1440),
      rateCostPerReceiver(1),
      rateCostPerMail(1),
      rateCostPerPost(5),
      getNewestLimit(200),
      notificationDelay(6)
{ }
