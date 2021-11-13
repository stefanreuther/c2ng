/**
  *  \file server/host/configuration.cpp
  *  \brief Structure server::host::Configuration
  */

#include "server/host/configuration.hpp"
#include "server/ports.hpp"

// Default constructor.
server::host::Configuration::Configuration()
    : timeScale(60),
      workDirectory(),
      binDirectory("."),
      useCron(true),
      unpackBackups(false),
      usersSeeTemporaryTurns(true),
      numMissedTurnsForKick(0),
      hostFileAddress(DEFAULT_ADDRESS, HOSTFILE_PORT),
      initialSuspend(0),
      maxStoredKeys(10),
      keyTitle(),
      keySecret()
{ }

// Convert time.
int32_t
server::host::Configuration::getUserTimeFromTime(Time_t t) const
{
    // ex getUserTimeFromTime
    return t*timeScale/60;
}
