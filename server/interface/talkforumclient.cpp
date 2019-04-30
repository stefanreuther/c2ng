/**
  *  \file server/interface/talkforumclient.cpp
  */

#include "server/interface/talkforumclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"

using afl::data::Segment;

server::interface::TalkForumClient::TalkForumClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::TalkForumClient::~TalkForumClient()
{ }

// FORUMADD [key:Str value:Str ...] (Talk Command)
// @retval FID new forum Id
int32_t
server::interface::TalkForumClient::add(afl::base::Memory<const String_t> config)
{
    Segment cmd;
    cmd.pushBackString("FORUMADD");
    while (const String_t* p = config.eat()) {
        cmd.pushBackString(*p);
    }
    return m_commandHandler.callInt(cmd);
}

// FORUMSET forum:FID [key:Str value:Str ...] (Talk Command)
void
server::interface::TalkForumClient::configure(int32_t fid, afl::base::Memory<const String_t> config)
{
    Segment cmd;
    cmd.pushBackString("FORUMSET");
    cmd.pushBackInteger(fid);
    while (const String_t* p = config.eat()) {
        cmd.pushBackString(*p);
    }
    m_commandHandler.callVoid(cmd);
}

// FORUMGET forum:FID key:Str (Talk Command)
// @rettype Str, GRID, TalkText, TalkPerm, Time, Int
afl::data::Value*
server::interface::TalkForumClient::getValue(int32_t fid, String_t keyName)
{
    return m_commandHandler.call(Segment().pushBackString("FORUMGET").pushBackInteger(fid).pushBackString(keyName));
}

// FORUMSTAT forum:FID (Talk Command)
// @retval TalkForumInfo information about forum
server::interface::TalkForum::Info
server::interface::TalkForumClient::getInfo(int32_t fid)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("FORUMSTAT").pushBackInteger(fid)));
    return unpackInfo(p.get());    
}

// FORUMMSTAT forum:FID... (Talk Command)
// @retval TalkForumInfo[] information about forum
void
server::interface::TalkForumClient::getInfo(afl::base::Memory<const int32_t> fids, afl::container::PtrVector<Info>& result)
{
    Segment cmd;
    cmd.pushBackString("FORUMMSTAT");
    while (const int32_t* p = fids.eat()) {
        cmd.pushBackInteger(*p);
    }

    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(cmd));
    afl::data::Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        if (a[i].isNull()) {
            result.pushBackNew(0);
        } else {
            result.pushBackNew(new Info(unpackInfo(a[i].getValue())));
        }
    }
}

// FORUMPERMS forum:FID [perm:Str ...] (Talk Command)
// @retval Int permissions
// FIXME: can we find a better interface?
int32_t
server::interface::TalkForumClient::getPermissions(int32_t fid, afl::base::Memory<const String_t> permissionList)
{
    Segment cmd;
    cmd.pushBackString("FORUMPERMS");
    cmd.pushBackInteger(fid);
    while (const String_t* p = permissionList.eat()) {
        cmd.pushBackString(*p);
    }
    return m_commandHandler.callInt(cmd);
}

// FORUMSIZE forum:FID (Talk Command)
server::interface::TalkForum::Size
server::interface::TalkForumClient::getSize(int32_t fid)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("FORUMSIZE").pushBackInteger(fid)));
    afl::data::Access a(p);

    Size result;
    result.numThreads       = a("threads").toInteger();
    result.numStickyThreads = a("stickythreads").toInteger();
    result.numMessages      = a("messages").toInteger();
    return result;
}

// FORUMLSTHREAD forum:FID [listParameters...] (Talk Command)
// @rettype Any, TID
afl::data::Value*
server::interface::TalkForumClient::getThreads(int32_t fid, const ListParameters& params)
{
    Segment cmd;
    cmd.pushBackString("FORUMLSTHREAD");
    cmd.pushBackInteger(fid);
    packListParameters(cmd, params);
    return m_commandHandler.call(cmd);
}

// FORUMLSSTICKY forum:FID [listParameters...] (Talk Command)
// @rettype Any, TID
afl::data::Value*
server::interface::TalkForumClient::getStickyThreads(int32_t fid, const ListParameters& params)
{
    Segment cmd;
    cmd.pushBackString("FORUMLSSTICKY");
    cmd.pushBackInteger(fid);
    packListParameters(cmd, params);
    return m_commandHandler.call(cmd);
}

// FORUMLSPOST forum:FID [listParameters...] (Talk Command)
afl::data::Value*
server::interface::TalkForumClient::getPosts(int32_t fid, const ListParameters& params)
{
    Segment cmd;
    cmd.pushBackString("FORUMLSPOST");
    cmd.pushBackInteger(fid);
    packListParameters(cmd, params);
    return m_commandHandler.call(cmd);
}

int32_t
server::interface::TalkForumClient::findForum(String_t key)
{
    return m_commandHandler.callInt(Segment().pushBackString("FORUMBYNAME").pushBackString(key));
}

server::interface::TalkForum::Info
server::interface::TalkForumClient::unpackInfo(const afl::data::Value* value)
{
    /* @type TalkForumInfo
       Description of a forum.

       @key name:Str (Forum name)
       @key parent:GRID (Containing group Id)
       @key description:Str (Description/subtitle, rendered using {RENDEROPTION})
       @key newsgroup:Str (Newsgroup name) */
    afl::data::Access a(value);
    Info result;
    result.name          = a("name").toString();
    result.parentGroup   = a("parent").toString();
    result.description   = a("description").toString();
    result.newsgroupName = a("newsgroup").toString();
    return result;
}

void
server::interface::TalkForumClient::packListParameters(afl::data::Segment& cmd, const ListParameters& params)
{
    /* @page pcc:talk:listparams, List Parameters
       @in pcc:resp
       Various {@group Talk Command}s return lists of things.
       These commands accept a common set of options to limit or modify the result.

       - <tt>LIMIT start count</tt>: return at most %count elements, starting at the %start'th (returns a list)
       - <tt>SIZE</tt>: return number of elements (integer)
       - <tt>CONTAINS n</tt>: returns 1 if the list contains element %n, otherwise 0 (integer)
       - <tt>SORT key</tt>: if a list is returned, sort it by the given key.
         Valid sort keys depend on the command.

       The result is always sorted in ascending order. */

    switch (params.mode) {
     case ListParameters::WantAll:
        break;
     case ListParameters::WantRange:
        cmd.pushBackString("LIMIT");
        cmd.pushBackInteger(params.start);
        cmd.pushBackInteger(params.count);
        break;
     case ListParameters::WantSize:
        cmd.pushBackString("SIZE");
        break;
     case ListParameters::WantMemberCheck:
        cmd.pushBackString("CONTAINS");
        cmd.pushBackInteger(params.item);
        break;
    }

    if (const String_t* p = params.sortKey.get()) {
        cmd.pushBackString("SORT");
        cmd.pushBackString(*p);
    }
}
