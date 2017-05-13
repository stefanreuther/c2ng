/**
  *  \file server/interface/talkthreadclient.cpp
  */

#include <memory>
#include "server/interface/talkthreadclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"
#include "server/interface/talkforumclient.hpp"

using afl::data::Segment;

server::interface::TalkThreadClient::TalkThreadClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::TalkThreadClient::~TalkThreadClient()
{ }

server::interface::TalkThread::Info
server::interface::TalkThreadClient::getInfo(int32_t threadId)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("THREADSTAT").pushBackInteger(threadId)));
    return unpackInfo(p.get());
}

void
server::interface::TalkThreadClient::getInfo(afl::base::Memory<const int32_t> threadIds, afl::container::PtrVector<Info>& result)
{
    Segment cmd;
    cmd.pushBackString("THREADMSTAT");
    while (const int32_t* p = threadIds.eat()) {
        cmd.pushBackInteger(*p);
    }

    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(cmd));
    afl::data::Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        if (const afl::data::Value* p = a[i].getValue()) {
            result.pushBackNew(new Info(unpackInfo(p)));
        } else {
            result.pushBackNew(0);
        }
    }
}

afl::data::Value*
server::interface::TalkThreadClient::getPosts(int32_t threadId, const ListParameters& params)
{
    Segment cmd;
    cmd.pushBackString("THREADLSPOST");
    cmd.pushBackInteger(threadId);
    TalkForumClient::packListParameters(cmd, params);
    return m_commandHandler.call(cmd);
}

void
server::interface::TalkThreadClient::setSticky(int32_t threadId, bool flag)
{
    m_commandHandler.callVoid(Segment().pushBackString("THREADSTICKY").pushBackInteger(threadId).pushBackInteger(flag));
}

int
server::interface::TalkThreadClient::getPermissions(int32_t threadId, afl::base::Memory<const String_t> permissionList)
{
    Segment cmd;
    cmd.pushBackString("THREADPERMS");
    cmd.pushBackInteger(threadId);
    while (const String_t* p = permissionList.eat()) {
        cmd.pushBackString(*p);
    }
    return m_commandHandler.callInt(cmd);
}

void
server::interface::TalkThreadClient::moveToForum(int32_t threadId, int32_t forumId)
{
    m_commandHandler.callVoid(Segment().pushBackString("THREADMV").pushBackInteger(threadId).pushBackInteger(forumId));
}

bool
server::interface::TalkThreadClient::remove(int32_t threadId)
{
    return m_commandHandler.callInt(Segment().pushBackString("THREADRM").pushBackInteger(threadId));
}

server::interface::TalkThread::Info
server::interface::TalkThreadClient::unpackInfo(const afl::data::Value* p)
{
    afl::data::Access a(p);

    Info result;
    result.subject = a("subject").toString();
    result.forumId = a("forum").toInteger();
    result.firstPostId = a("firstpost").toInteger();
    result.lastPostId = a("lastpost").toInteger();
    result.lastTime = a("lasttime").toInteger();
    result.isSticky = a("sticky").toInteger() != 0;
    return result;
}
