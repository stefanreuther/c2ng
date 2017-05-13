/**
  *  \file server/talk/session.cpp
  */

#include <stdexcept>
#include "server/talk/session.hpp"
#include "server/talk/root.hpp"
#include "server/errors.hpp"

server::talk::Session::Session()
    : m_renderOptions()
{ }

server::talk::Session::~Session()
{ }

server::talk::render::Options&
server::talk::Session::renderOptions()
{
    // ex TalkConnection::getRenderState (sort-of)
    return m_renderOptions;
}

// /** Check privilege string.
//     \param privString Privilege string from database
//     \return true iff user is allowed to see the protected item */
bool
server::talk::Session::hasPrivilege(String_t privString, Root& root)
{
    // ex TalkConnection::hasPrivilege
    // FIXME: replace root parameter by m_root member.
    // This also allows functions that take a Root+Session to be simplified.
    return isAdmin() || root.checkUserPermission(privString, getUser());
}

// /** Check privilege string. Throws std::runtime_error if user doesn't have permissions.
//     \param privString Privilege string from database */
void
server::talk::Session::checkPrivilege(String_t privString, Root& root)
{
    // ex TalkConnection::checkPrivilege
    if (!hasPrivilege(privString, root)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
}
