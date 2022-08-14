/**
  *  \file server/interface/hostfileclient.cpp
  *  \brief Class server::interface::HostFileClient
  */

#include "server/interface/hostfileclient.hpp"
#include "afl/data/access.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/types.hpp"

using afl::data::Access;
using afl::data::Segment;
using afl::data::Value;

server::interface::HostFileClient::HostFileClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

String_t
server::interface::HostFileClient::getFile(String_t fileName)
{
    return m_commandHandler.callString(Segment().pushBackString("GET").pushBackString(fileName));
}

void
server::interface::HostFileClient::getDirectoryContent(String_t dirName, InfoVector_t& result)
{
    std::auto_ptr<Value> p(m_commandHandler.call(Segment().pushBackString("LS").pushBackString(dirName)));
    unpackInfos(p.get(), result);
}

server::interface::HostFile::Info
server::interface::HostFileClient::getFileInformation(String_t fileName)
{
    std::auto_ptr<Value> p(m_commandHandler.call(Segment().pushBackString("STAT").pushBackString(fileName)));
    return unpackInfo(p.get());
}

void
server::interface::HostFileClient::getPathDescription(String_t dirName, InfoVector_t& result)
{
    std::auto_ptr<Value> p(m_commandHandler.call(Segment().pushBackString("PSTAT").pushBackString(dirName)));
    unpackInfos(p.get(), result);
}

server::interface::HostFile::Info
server::interface::HostFileClient::unpackInfo(const afl::data::Value* p)
{
    Info result;
    Access a(p);

    // Decode FileBase part
    static_cast<FileBase::Info&>(result) = FileBaseClient::unpackInfo(p);

    // Decode remainder
    // - name
    result.name = a("name").toString();

    // - label. If the value is missing or invalid, leave it at default, do not make this fatal.
    parseLabel(a("label").toString(), result.label);

    // - optional parts
    result.turnNumber = toOptionalInteger(a("turn").getValue());
    result.slotId     = toOptionalInteger(a("slot").getValue());
    result.slotName   = toOptionalString(a("slotname").getValue());
    result.gameId     = toOptionalInteger(a("game").getValue());
    result.gameName   = toOptionalString(a("gamename").getValue());
    result.toolName   = toOptionalString(a("toolname").getValue());
    return result;
}

void
server::interface::HostFileClient::unpackInfos(const afl::data::Value* p, InfoVector_t& result)
{
    Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i+1 < n; i += 2) {
        Info info = unpackInfo(a[i+1].getValue());
        info.name = a[i].toString();
        result.push_back(info);
    }
}
