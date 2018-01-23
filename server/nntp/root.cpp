/**
  *  \file server/nntp/root.cpp
  *  \brief Class server::nntp::Root
  */

#include "server/nntp/root.hpp"
#include "afl/net/reconnectable.hpp"

server::nntp::Root::Root(afl::net::CommandHandler& talk, const String_t& baseUrl)
    : m_talk(talk),
      m_baseUrl(baseUrl),
      m_log(),
      m_idCounter(0)
{ }

afl::sys::Log&
server::nntp::Root::log()
{
    return m_log;
}

uint32_t
server::nntp::Root::allocateId()
{
    return ++m_idCounter;
}

afl::net::CommandHandler&
server::nntp::Root::talk()
{
    return m_talk;
}

void
server::nntp::Root::configureReconnect()
{
    if (afl::net::Reconnectable* rc = dynamic_cast<afl::net::Reconnectable*>(&m_talk)) {
        rc->setReconnectMode(afl::net::Reconnectable::Once);
    }
}

const String_t&
server::nntp::Root::getBaseUrl() const
{
    return m_baseUrl;
}
