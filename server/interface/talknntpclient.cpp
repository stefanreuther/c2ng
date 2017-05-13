/**
  *  \file server/interface/talknntpclient.cpp
  */

#include <memory>
#include "server/interface/talknntpclient.hpp"
#include "server/types.hpp"
#include "afl/data/access.hpp"

using afl::data::Segment;

server::interface::TalkNNTPClient::TalkNNTPClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::TalkNNTPClient::~TalkNNTPClient()
{ }

String_t
server::interface::TalkNNTPClient::checkUser(String_t loginName, String_t password)
{
    return m_commandHandler.callString(Segment().pushBackString("NNTPUSER").pushBackString(loginName).pushBackString(password));
}

void
server::interface::TalkNNTPClient::listNewsgroups(afl::container::PtrVector<Info>& result)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("NNTPLIST")));
    afl::data::Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        result.pushBackNew(new Info(unpackInfo(a[i].getValue())));
    }
}

server::interface::TalkNNTP::Info
server::interface::TalkNNTPClient::findNewsgroup(String_t newsgroupName)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("NNTPFINDNG").pushBackString(newsgroupName)));
    return unpackInfo(p.get());
}

int32_t
server::interface::TalkNNTPClient::findMessage(String_t rfcMsgId)
{
    return m_commandHandler.callInt(Segment().pushBackString("NNTPFINDMID").pushBackString(rfcMsgId));
}

void
server::interface::TalkNNTPClient::listMessages(int32_t forumId, afl::data::IntegerList_t& result)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("NNTPFORUMLS").pushBackInteger(forumId)));
    afl::data::Access(p).toIntegerList(result);
}

afl::data::Hash::Ref_t
server::interface::TalkNNTPClient::getMessageHeader(int32_t messageId)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("NNTPPOSTHEAD").pushBackInteger(messageId)));

    // FIXME: this conversion-to-hash should be an operation in Access for efficiency
    afl::data::Hash::Ref_t result = afl::data::Hash::create();
    afl::data::Access a(p);
    afl::data::StringList_t keys;
    a.getHashKeys(keys);
    for (size_t i = 0, n = keys.size(); i < n; ++i) {
        result->setNew(keys[i], makeStringValue(a(keys[i]).toString()));
    }
    return result;
}

void
server::interface::TalkNNTPClient::getMessageHeader(afl::base::Memory<const int32_t> messageIds, afl::data::Segment& results)
{
    Segment cmd;
    cmd.pushBackString("NNTPPOSTMHEAD");
    while (const int32_t* p = messageIds.eat()) {
        cmd.pushBackInteger(*p);
    }

    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(cmd));
    afl::data::Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        results.pushBack(a[i].getValue());
    }
}

void
server::interface::TalkNNTPClient::listNewsgroupsByGroup(String_t groupId, afl::data::StringList_t& result)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("NNTPGROUPLS").pushBackString(groupId)));
    afl::data::Access(p).toStringList(result);
}

server::interface::TalkNNTP::Info
server::interface::TalkNNTPClient::unpackInfo(const afl::data::Value* p)
{
    afl::data::Access a(p);
    Info result;
    result.forumId             = a("id").toInteger();
    result.newsgroupName       = a("newsgroup").toString();
    result.firstSequenceNumber = a("firstSeq").toInteger();
    result.lastSequenceNumber  = a("lastSeq").toInteger();
    result.writeAllowed        = a("writeAllowed").toInteger() != 0;
    result.description         = a("description").toString();
    return result;
}
