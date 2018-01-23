/**
  *  \file server/talk/session.cpp
  *  \brief Class server::talk::Session
  */

#include <stdexcept>
#include "server/talk/session.hpp"
#include "server/talk/root.hpp"
#include "server/errors.hpp"

// Default constructor.
server::talk::Session::Session()
    : m_renderOptions()
{ }

// Destructor.
server::talk::Session::~Session()
{ }

// Access render options.
server::talk::render::Options&
server::talk::Session::renderOptions()
{
    // ex TalkConnection::getRenderState (sort-of)
    return m_renderOptions;
}

// Check permission.
bool
server::talk::Session::hasPermission(String_t privString, Root& root) const
{
    // ex TalkConnection::hasPrivilege
    return isAdmin() || root.checkUserPermission(privString, getUser());
}

// Check permission, throw.
void
server::talk::Session::checkPermission(String_t privString, Root& root) const
{
    // ex TalkConnection::checkPrivilege
    if (!hasPermission(privString, root)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
}
