/**
  *  \file server/host/session.cpp
  *  \brief Class server::host::Session
  */

#include <stdexcept>
#include "server/host/session.hpp"
#include "server/errors.hpp"

// Constructor.
server::host::Session::Session()
{ }

// Destructor.
server::host::Session::~Session()
{ }

// Check permissions.
void
server::host::Session::checkPermission(Game& g, Game::PermissionLevel level) const
{
    // ex HostConnection::checkPermission
    if (!g.hasPermission(getUser(), level)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
}
