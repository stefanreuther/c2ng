/**
  *  \file server/interface/talkuserclient.cpp
  */

#include "server/interface/talkuserclient.hpp"
#include "server/interface/talkforumclient.hpp"

using afl::data::Segment;

server::interface::TalkUserClient::TalkUserClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::TalkUserClient::~TalkUserClient()
{ }

// USERNEWSRC action:Str [range...] (Talk Command)
afl::data::Value*
server::interface::TalkUserClient::accessNewsrc(Modification modif, Result res, afl::base::Memory<const Selection> selections, afl::base::Memory<const int32_t> posts)
{
    Segment cmd;
    cmd.pushBackString("USERNEWSRC");
    switch (modif) {
     case NoModification:                                    break;
     case MarkRead:        cmd.pushBackString("SET");        break;
     case MarkUnread:      cmd.pushBackString("CLEAR");      break;
    }
    switch (res) {
     case NoResult:                                          break;
     case GetAll:          cmd.pushBackString("GET");        break;
     case CheckIfAnyRead:  cmd.pushBackString("ANY");        break;
     case CheckIfAllRead:  cmd.pushBackString("ALL");        break;
     case GetFirstRead:    cmd.pushBackString("FIRSTSET");   break;
     case GetFirstUnread:  cmd.pushBackString("FIRSTCLEAR"); break;
    }
    packSelections(cmd, selections);
    if (!posts.empty()) {
        cmd.pushBackString("POST");
        while (const int32_t* p = posts.eat()) {
            cmd.pushBackInteger(*p);
        }
    }
    return m_commandHandler.call(cmd);
}

// USERWATCH [THREAD n:TID] [FORUM n:FID]... (Talk Command)
void
server::interface::TalkUserClient::watch(afl::base::Memory<const Selection> selections)
{
    Segment cmd;
    cmd.pushBackString("USERWATCH");
    packSelections(cmd, selections);
    m_commandHandler.callVoid(cmd);
}

// USERUNWATCH [THREAD n:TID] [FORUM n:FID]... (Talk Command)
void
server::interface::TalkUserClient::unwatch(afl::base::Memory<const Selection> selections)
{
    Segment cmd;
    cmd.pushBackString("USERUNWATCH");
    packSelections(cmd, selections);
    m_commandHandler.callVoid(cmd);
}

// USERMARKSEEN [THREAD n:TID] [FORUM n:FID]... (Talk Command)
void
server::interface::TalkUserClient::markSeen(afl::base::Memory<const Selection> selections)
{
    Segment cmd;
    cmd.pushBackString("USERMARKSEEN");
    packSelections(cmd, selections);
    m_commandHandler.callVoid(cmd);
}

// USERLSWATCHEDTHREADS [listParameters...] (Talk Command)
afl::data::Value*
server::interface::TalkUserClient::getWatchedThreads(const ListParameters& params)
{
    Segment cmd;
    cmd.pushBackString("USERLSWATCHEDTHREADS");
    TalkForumClient::packListParameters(cmd, params);
    return m_commandHandler.call(cmd);
}

// USERLSWATCHEDFORUMS [listParameters...] (Talk Command)
afl::data::Value*
server::interface::TalkUserClient::getWatchedForums(const ListParameters& params)
{
    Segment cmd;
    cmd.pushBackString("USERLSWATCHEDFORUMS");
    TalkForumClient::packListParameters(cmd, params);
    return m_commandHandler.call(cmd);
}

// USERLSPOSTED user:UID [listParameters...] (Talk Command)
afl::data::Value*
server::interface::TalkUserClient::getPostedMessages(String_t user, const ListParameters& params)
{
    Segment cmd;
    cmd.pushBackString("USERLSPOSTED");
    cmd.pushBackString(user);
    TalkForumClient::packListParameters(cmd, params);
    return m_commandHandler.call(cmd);
}

afl::data::Value*
server::interface::TalkUserClient::getCrosspostToGameCandidates(const ListParameters& params)
{
    Segment cmd;
    cmd.pushBackString("USERLSCROSS");
    TalkForumClient::packListParameters(cmd, params);
    return m_commandHandler.call(cmd);
}

void
server::interface::TalkUserClient::packSelections(afl::data::Segment& cmd, afl::base::Memory<const Selection> selections)
{
    while (const Selection* p = selections.eat()) {
        switch (p->scope) {
         case ForumScope:
            cmd.pushBackString("FORUM");
            cmd.pushBackInteger(p->id);
            break;
         case ThreadScope:
            cmd.pushBackString("THREAD");
            cmd.pushBackInteger(p->id);
            break;
         case RangeScope:
            cmd.pushBackString("RANGE");
            cmd.pushBackInteger(p->id);
            cmd.pushBackInteger(p->lastId);
            break;
        }
    }
}
