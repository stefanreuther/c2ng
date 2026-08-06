/**
  *  \file server/interface/fileshareclient.cpp
  *  \brief Class server::interface::FileShareClient
  */

#include "server/interface/fileshareclient.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "server/interface/filegameclient.hpp"

using afl::data::Access;
using afl::data::Segment;
using afl::data::Value;

server::interface::FileShareClient::FileShareClient(afl::net::CommandHandler& hdl)
    : m_commandHandler(hdl)
{ }

int32_t
server::interface::FileShareClient::getSequenceNumber(String_t userId)
{
    return m_commandHandler.callInt(Segment().pushBackString("SHARESEQ").pushBackString(userId));
}

void
server::interface::FileShareClient::listDirectories(String_t userId, afl::container::PtrVector<Info>& result)
{
    std::auto_ptr<Value> p(m_commandHandler.call(Segment().pushBackString("SHARELS").pushBackString(userId)));
    Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        result.pushBackNew(unpackInfo(a[i].getValue()).release());
    }
}

void
server::interface::FileShareClient::listGameInfo(String_t userId, afl::container::PtrVector<FileGame::GameInfo>& result)
{
    std::auto_ptr<Value> p(m_commandHandler.call(Segment().pushBackString("SHARELSGAME").pushBackString(userId)));
    Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        FileGame::GameInfo* gi = result.pushBackNew(new FileGame::GameInfo());
        FileGameClient::unpackGameInfo(*gi, a[i].getValue());
    }
}

std::auto_ptr<server::interface::FileShare::Info>
server::interface::FileShareClient::unpackInfo(const afl::data::Value* p)
{
    std::auto_ptr<Info> result(new Info());
    result->pathName = Access(p)("path").toString();
    result->owner = Access(p)("owner").toString();
    result->seqNr = Access(p)("seqNr").toInteger();
    return result;
}
