/**
  *  \file server/user/root.cpp
  */

#include "server/user/root.hpp"
#include "server/user/token.hpp"
#include "afl/string/format.hpp"

// Constructor.
server::user::Root::Root(afl::net::CommandHandler& db, server::common::IdGenerator& gen, PasswordEncrypter& encrypter, const Configuration& config)
    : server::common::Root(db),
      m_log(),
      m_db(db),
      m_generator(gen),
      m_encrypter(encrypter),
      m_config(config)
{ }

// Destructor.
server::user::Root::~Root()
{ }

// Access logger.
afl::sys::Log&
server::user::Root::log()
{
    return m_log;
}

server::common::IdGenerator&
server::user::Root::generator()
{
    return m_generator;
}

server::user::PasswordEncrypter&
server::user::Root::encrypter()
{
    return m_encrypter;
}

server::Time_t
server::user::Root::getTime()
{
    return packTime(afl::sys::Time::getCurrentTime());
}

const server::user::Configuration&
server::user::Root::config() const
{
    return m_config;
}

afl::net::redis::StringSetKey
server::user::Root::allTokens()
{
    return afl::net::redis::StringSetKey(m_db, "token:all");
}

server::user::Token
server::user::Root::tokenById(String_t token)
{
    return Token(afl::net::redis::HashKey(m_db, "token:t:" + token));
}

String_t
server::user::Root::allocateUserId()
{
    // Although user Ids are numeric, we treat them as strings almost anywhere.
    // This is the only place that treats a user Ids as a number.
    return afl::string::Format("%d", ++afl::net::redis::IntegerKey(m_db, "user:uid"));
}

afl::net::redis::StringSetKey
server::user::Root::allUsers()
{
    return afl::net::redis::StringSetKey(m_db, "user:all");
}

// Access default profile copy.
afl::net::redis::HashKey
server::user::Root::defaultProfileCopy()
{
    return afl::net::redis::HashKey(m_db, "default:profilecopy");
}
