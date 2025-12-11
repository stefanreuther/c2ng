/**
  *  \file server/interface/filesnapshotclient.cpp
  *  \brief Class server::interface::FileSnapshotClient
!  */

#include "server/interface/filesnapshotclient.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"

using afl::data::Segment;

server::interface::FileSnapshotClient::FileSnapshotClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::FileSnapshotClient::~FileSnapshotClient()
{ }

void
server::interface::FileSnapshotClient::createSnapshot(String_t name)
{
    m_commandHandler.callVoid(Segment().pushBackString("SNAPSHOTADD").pushBackString(name));
}

void
server::interface::FileSnapshotClient::copySnapshot(String_t oldName, String_t newName)
{
    m_commandHandler.callVoid(Segment().pushBackString("SNAPSHOTCP").pushBackString(oldName).pushBackString(newName));
}

void
server::interface::FileSnapshotClient::removeSnapshot(String_t name)
{
    m_commandHandler.callVoid(Segment().pushBackString("SNAPSHOTRM").pushBackString(name));
}

void
server::interface::FileSnapshotClient::listSnapshots(afl::data::StringList_t& out)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("SNAPSHOTLS")));
    afl::data::Access(p.get()).toStringList(out);
}
