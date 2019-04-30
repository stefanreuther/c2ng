/**
  *  \file server/interface/userdataclient.cpp
  *  \brief Class server::interface::UserDataClient
  */

#include "server/interface/userdataclient.hpp"
#include "afl/data/segment.hpp"

using afl::data::Segment;

server::interface::UserDataClient::UserDataClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

void
server::interface::UserDataClient::set(String_t userId, String_t key, String_t value)
{
    m_commandHandler.callVoid(Segment().pushBackString("USET").pushBackString(userId).pushBackString(key).pushBackString(value));
}

String_t
server::interface::UserDataClient::get(String_t userId, String_t key)
{
    return m_commandHandler.callString(Segment().pushBackString("UGET").pushBackString(userId).pushBackString(key));
}
