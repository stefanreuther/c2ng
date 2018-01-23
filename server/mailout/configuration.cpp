/**
  *  \file server/mailout/configuration.cpp
  *  \brief Structure server::mailout::Configuration
  */

#include "server/mailout/configuration.hpp"

server::mailout::Configuration::Configuration()
    : baseUrl("unconfigured"),
      confirmationKey(),
      maximumAge(24*60*32),
      useTransmitter(true)
{ }
