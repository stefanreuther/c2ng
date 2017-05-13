/**
  *  \file server/common/session.cpp
  *  \brief Class server::common::Session
  */

#include <stdexcept>
#include "server/common/session.hpp"
#include "server/errors.hpp"

// Constructor.
server::common::Session::Session()
    : m_user()
{ }

// Destructor.
server::common::Session::~Session()
{ }

// Set the user.
void
server::common::Session::setUser(String_t user)
{
    m_user = user;
}

// Get current user.
String_t
server::common::Session::getUser() const
{
    // ex TalkConnection::getUser
    return m_user;
}

// Check for admin permissions.
bool
server::common::Session::isAdmin() const
{
    // ex TalkConnection::isAdmin
    return m_user.empty();
}

// Check for admin permissions.
void
server::common::Session::checkAdmin() const
{
    // ex TalkConnection::checkAdmin
    if (!isAdmin()) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
}

// Check for user context.
void
server::common::Session::checkUser() const
{
    // ex TalkConnection::checkUser
    if (isAdmin()) {
        throw std::runtime_error(MUST_HAVE_USER_CONTEXT);
    }
}
