/**
  *  \file server/interface/hostrankingclient.cpp
  *  \brief Class server::interface::HostRankingClient
  */

#include "server/interface/hostrankingclient.hpp"
#include "afl/data/segment.hpp"

server::interface::HostRankingClient::HostRankingClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::Value_t*
server::interface::HostRankingClient::getUserList(const ListRequest& req)
{
    afl::data::Segment cmd;
    cmd.pushBackString("RANKLIST");
    if (const String_t* sort = req.sortField.get()) {
        cmd.pushBackString("SORT");
        cmd.pushBackString(*sort);
    }
    if (req.sortReverse) {
        cmd.pushBackString("REVERSE");
    }
    if (!req.fieldsToGet.empty()) {
        cmd.pushBackString("FIELDS");
        cmd.pushBackElements(req.fieldsToGet);
    }

    return m_commandHandler.call(cmd);
}
