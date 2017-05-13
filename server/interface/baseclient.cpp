/**
  *  \file server/interface/baseclient.cpp
  */

#include "server/interface/baseclient.hpp"
#include "afl/data/segment.hpp"

using afl::data::Segment;

server::interface::BaseClient::BaseClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::BaseClient::~BaseClient()
{ }

String_t
server::interface::BaseClient::ping()
{
    return m_commandHandler.callString(Segment().pushBackString("PING"));
}

void
server::interface::BaseClient::setUserContext(String_t user)
{
    m_commandHandler.callVoid(Segment().pushBackString("USER").pushBackString(user));
}
