/**
  *  \file server/user/token.cpp
  *  \brief Class server::user::Token
  */

#include "server/user/token.hpp"

server::user::Token::Token(afl::net::redis::HashKey key)
    : m_key(key)
{ }

afl::net::redis::StringField
server::user::Token::userId()
{
    return m_key.stringField("user");
}

afl::net::redis::StringField
server::user::Token::tokenType()
{
    return m_key.stringField("type");
}

afl::net::redis::IntegerField
server::user::Token::validUntil()
{
    return m_key.intField("until");
}

void
server::user::Token::remove()
{
    m_key.remove();
}
