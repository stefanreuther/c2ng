/**
  *  \file server/interface/hostspecificationclient.cpp
  *  \brief Class server::interface::HostSpecificationClient
  */

#include "server/interface/hostspecificationclient.hpp"

server::interface::HostSpecificationClient::HostSpecificationClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::Value_t*
server::interface::HostSpecificationClient::getShiplistData(String_t shiplistId, Format format, const afl::data::StringList_t& keys)
{
    return m_commandHandler.call(afl::data::Segment()
                                 .pushBackString("SPECSHIPLIST")
                                 .pushBackString(shiplistId)
                                 .pushBackString(formatFormat(format))
                                 .pushBackElements(keys));
}

server::Value_t*
server::interface::HostSpecificationClient::getGameData(int32_t gameId, Format format, const afl::data::StringList_t& keys)
{
    return m_commandHandler.call(afl::data::Segment()
                                 .pushBackString("SPECGAME")
                                 .pushBackInteger(gameId)
                                 .pushBackString(formatFormat(format))
                                 .pushBackElements(keys));
}
