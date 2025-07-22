/**
  *  \file server/interface/talkpostclient.cpp
  *  \brief Class server::interface::TalkPostClient
  */

#include "server/interface/talkpostclient.hpp"
#include "afl/data/access.hpp"
#include "server/interface/talkrenderclient.hpp"

using afl::data::Segment;

server::interface::TalkPostClient::TalkPostClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

// POSTNEW forum:FID subj:Str text:TalkText [USER user:UID] [READPERM rp:Str] [ANSWERPERM ap:Str]
// @retval MID new message Id
int32_t
server::interface::TalkPostClient::create(int32_t forumId, String_t subject, String_t text, const CreateOptions& options)
{
    Segment cmd;
    cmd.pushBackString("POSTNEW");
    cmd.pushBackInteger(forumId);
    cmd.pushBackString(subject);
    cmd.pushBackString(text);
    if (const String_t* p = options.userId.get()) {
        cmd.pushBackString("USER");
        cmd.pushBackString(*p);
    }
    if (const String_t* p = options.readPermissions.get()) {
        cmd.pushBackString("READPERM");
        cmd.pushBackString(*p);
    }
    if (const String_t* p = options.answerPermissions.get()) {
        cmd.pushBackString("ANSWERPERM");
        cmd.pushBackString(*p);
    }
    for (size_t i = 0; i < options.alsoPostTo.size(); ++i) {
        cmd.pushBackString("ALSO");
        cmd.pushBackInteger(options.alsoPostTo[i]);
    }
    return m_commandHandler.callInt(cmd);
}

// POSTREPLY parent:MID subj:Str text:TalkText [USER user:UID]
// @retval MID new message Id
int32_t
server::interface::TalkPostClient::reply(int32_t parentPostId, String_t subject, String_t text, const ReplyOptions& options)
{
    Segment cmd;
    cmd.pushBackString("POSTREPLY");
    cmd.pushBackInteger(parentPostId);
    cmd.pushBackString(subject);
    cmd.pushBackString(text);
    if (const String_t* p = options.userId.get()) {
        cmd.pushBackString("USER");
        cmd.pushBackString(*p);
    }
    return m_commandHandler.callInt(cmd);
}

// POSTEDIT msg:MID subj:Str text:TalkText
void
server::interface::TalkPostClient::edit(int32_t postId, String_t subject, String_t text)
{
    m_commandHandler.callVoid(Segment().pushBackString("POSTEDIT").pushBackInteger(postId).pushBackString(subject).pushBackString(text));
}

// POSTRENDER msg:MID [renderOptions...]
// @retval Str rendered posting
String_t
server::interface::TalkPostClient::render(int32_t postId, const TalkRender::Options& options)
{
    Segment cmd;
    cmd.pushBackString("POSTRENDER");
    cmd.pushBackInteger(postId);
    TalkRenderClient::packOptions(cmd, options);
    return m_commandHandler.callString(cmd);
}

// POSTMRENDER msg:MID...
// @retval StrList rendered postings
void
server::interface::TalkPostClient::render(afl::base::Memory<const int32_t> postIds, afl::data::StringList_t& result)
{
    Segment cmd;
    cmd.pushBackString("POSTMRENDER");
    while (const int32_t* p = postIds.eat()) {
        // FIXME: make this a function in Segment()?
        cmd.pushBackInteger(*p);
    }

    // FIXME: should we preserve null values?
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(cmd));
    afl::data::Access(p).toStringList(result);
}

// POSTSTAT msg:MID
// @retval TalkPostInfo information about posting
server::interface::TalkPost::Info
server::interface::TalkPostClient::getInfo(int32_t postId)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("POSTSTAT").pushBackInteger(postId)));
    return unpackInfo(p.get());
}

// POSTMSTAT msg:MID...
// @retval TalkPostInfo[]
void
server::interface::TalkPostClient::getInfo(afl::base::Memory<const int32_t> postIds, afl::container::PtrVector<Info>& result)
{
    Segment cmd;
    cmd.pushBackString("POSTMSTAT");
    while (const int32_t* p = postIds.eat()) {
        // FIXME: make this a function in Segment()?
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

// POSTGET msg:MID key:Str
// @retval Any result (string, Id, Time, etc.)
String_t
server::interface::TalkPostClient::getHeaderField(int32_t postId, String_t fieldName)
{
    return m_commandHandler.callString(Segment().pushBackString("POSTGET").pushBackInteger(postId).pushBackString(fieldName));
}

// POSTRM msg:MID
// @retval Int 1=removed, 0=not removed, posting did not exist
bool
server::interface::TalkPostClient::remove(int32_t postId)
{
    return m_commandHandler.callInt(Segment().pushBackString("POSTRM").pushBackInteger(postId));
}

// POSTLSNEW n:Int (Talk Command)
void
server::interface::TalkPostClient::getNewest(int count, afl::data::IntegerList_t& postIds)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("POSTLSNEW").pushBackInteger(count)));
    afl::data::Access(p).toIntegerList(postIds);
}

server::interface::TalkPost::Info
server::interface::TalkPostClient::unpackInfo(const afl::data::Value* value)
{
    afl::data::Access a(value);
    Info result;
    result.threadId     = a("thread").toInteger();
    result.parentPostId = a("parent").toInteger();
    result.postTime     = a("time").toInteger();
    result.editTime     = a("edittime").toInteger();
    result.author       = a("author").toString();
    result.subject      = a("subject").toString();
    result.rfcMessageId = a("msgid").toString();
    return result;
}
