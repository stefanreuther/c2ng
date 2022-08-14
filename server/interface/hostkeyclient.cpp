/**
  *  \file server/interface/hostkeyclient.cpp
  *  \brief Class server::interface::HostKeyClient
  */

#include "server/interface/hostkeyclient.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"

using afl::data::Access;
using afl::data::Segment;

server::interface::HostKeyClient::HostKeyClient(afl::net::CommandHandler& commandHandler)
    : HostKey(),
      m_commandHandler(commandHandler)
{ }

server::interface::HostKeyClient::~HostKeyClient()
{ }

void
server::interface::HostKeyClient::listKeys(Infos_t& out)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("KEYLS")));
    Access a(p.get());
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        if (const Value_t* e = a[i].getValue()) {
            out.push_back(unpackInfo(e));
        }
    }
}

String_t
server::interface::HostKeyClient::getKey(String_t keyId)
{
    return m_commandHandler.callString(Segment().pushBackString("KEYGET").pushBackString(keyId));
}

server::interface::HostKey::Info
server::interface::HostKeyClient::unpackInfo(const Value_t* p)
{
    Info result;
    Access a(p);
    result.keyId        = a("id").toString();
    result.isRegistered = a("reg").toInteger() != 0;
    result.isServerKey  = a("server").toInteger() != 0;
    result.label1       = a("key1").toString();
    result.label2       = a("key2").toString();

    result.filePathName = toOptionalString(a("filePathName").getValue());
    result.fileUseCount = toOptionalInteger(a("fileUseCount").getValue());
    result.lastGame     = toOptionalInteger(a("game").getValue());
    result.lastGameName = toOptionalString(a("gameName").getValue());
    result.gameUseCount = toOptionalInteger(a("gameUseCount").getValue());
    result.gameLastUsed = toOptionalInteger(a("gameLastUsed").getValue());

    return result;
}
