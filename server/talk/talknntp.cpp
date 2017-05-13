/**
  *  \file server/talk/talknntp.cpp
  */

#include <stdexcept>
#include "server/talk/talknntp.hpp"
#include "server/errors.hpp"
#include "afl/checksums/md5.hpp"
#include "afl/charset/base64.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/user.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"
#include "afl/string/parse.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/accesschecker.hpp"
#include "afl/data/hashvalue.hpp"
#include "server/talk/group.hpp"

server::talk::TalkNNTP::TalkNNTP(Session& session, Root& root)
    : m_session(session), m_root(root)
{ }

server::talk::TalkNNTP::~TalkNNTP()
{ }

String_t
server::talk::TalkNNTP::checkUser(String_t loginName, String_t password)
{
    // ex doNntpUser
    // Check user name
    const String_t userId = m_root.getUserIdFromLogin(loginName);
    if (userId.empty()) {
        throw std::runtime_error(INVALID_USERNAME);
    }

    // Get their password
    String_t correctHash = User(m_root, userId).passwordHash().get();
    if (correctHash.empty()) {
        throw std::runtime_error(INVALID_PASSWORD);
    }

    // Validate
    afl::checksums::MD5 ctx;
    ctx.add(afl::string::toBytes(m_root.config().userKey));
    ctx.add(afl::string::toBytes(password));

    uint8_t hashBuffer[afl::checksums::MD5::MAX_HASH_SIZE];
    String_t userHash = "1," + afl::string::fromBytes(afl::charset::Base64().encode(afl::string::toMemory(afl::string::fromBytes(ctx.getHash(hashBuffer)))));
    while (userHash.size() > 0 && userHash[userHash.size()-1] == '=') {
        userHash.erase(userHash.size()-1);
    }
    if (userHash == correctHash) {
        return userId;
    } else {
        throw std::runtime_error(INVALID_PASSWORD);
    }
}

void
server::talk::TalkNNTP::listNewsgroups(afl::container::PtrVector<Info>& result)
{
    // ex doNntpList
    m_session.checkUser();

    // Get list of newsgroups
    afl::data::StringList_t list;
    m_root.newsgroupMap().getAll(list);

    // Build result
    render::Context ctx(m_session.getUser());
    render::Options opts;
    opts.setFormat("text");
    for (afl::data::StringList_t::size_type i = 0, n = list.size(); i < n; i += 2) {
        String_t ng = list[i];
        int32_t fid;
        if (afl::string::strToInteger(list[i+1], fid)) {
            Forum f(m_root, fid);
            if (m_session.hasPrivilege(f.readPermissions().get(), m_root)) {
                result.pushBackNew(new Info(f.describeAsNewsgroup(ctx, opts, m_root, m_session)));
            }
        }
    }
}

server::interface::TalkNNTP::Info
server::talk::TalkNNTP::findNewsgroup(String_t newsgroupName)
{
    // ex doNntpFindNG
    // @change The checkUser() is not in PCC2 c2talk.
    // It is required to have the render::Context see a valid user.
    m_session.checkUser();

    int32_t fid = m_root.newsgroupMap().intField(newsgroupName).get();
    if (fid == 0) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    } else {
        Forum f(m_root, fid);
        if (m_session.hasPrivilege(f.readPermissions().get(), m_root)) {
            render::Context ctx(m_session.getUser());
            render::Options opts;
            opts.setFormat("text");
            return f.describeAsNewsgroup(ctx, opts, m_root, m_session);
        } else {
            throw std::runtime_error(PERMISSION_DENIED);
        }
    }
}

int32_t
server::talk::TalkNNTP::findMessage(String_t rfcMsgId)
{
    // ex doNntpFindMID
    // Look it up
    int32_t result = Message::lookupRfcMessageId(m_root, rfcMsgId);
    if (result == 0) {
        throw std::runtime_error(MESSAGE_NOT_FOUND);
    }
    return result;
}

void
server::talk::TalkNNTP::listMessages(int32_t forumId, afl::data::IntegerList_t& result)
{
    // ex doNntpForumList
    Forum f(m_root, forumId);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }

    // Get result
    afl::net::redis::SortOperation op(f.messages().sort());
    Message::applySortBySequenceMap(m_root, op);
    op.getResult(result);
}

afl::data::Hash::Ref_t
server::talk::TalkNNTP::getMessageHeader(int32_t messageId)
{
    // ex doNntpPostHead
    // Must have a user because getRfcHeader will return a user's email address
    m_session.checkUser();

    Message msg(m_root, messageId);
    if (!msg.exists()) {
        throw std::runtime_error(MESSAGE_NOT_FOUND);
    }
    AccessChecker(m_root, m_session).checkMessage(msg);

    // Return result
    return msg.getRfcHeader(m_root);
}

void
server::talk::TalkNNTP::getMessageHeader(afl::base::Memory<const int32_t> messageIds, afl::data::Segment& results)
{
    // ex doNntpPostMHead
    // Must have a user because getRfcHeader will return a user's email address
    m_session.checkUser();

    AccessChecker checker(m_root, m_session);
    while (const int32_t* p = messageIds.eat()) {
        const int32_t messageId = *p;
        Message msg(m_root, messageId);
        if (!msg.exists() || !checker.isAllowed(msg)) {
            results.pushBackNew(0);
        } else {
            results.pushBackNew(new afl::data::HashValue(msg.getRfcHeader(m_root)));
        }
    }
}

void
server::talk::TalkNNTP::listNewsgroupsByGroup(String_t groupId, afl::data::StringList_t& result)
{
    // ex doNntpGroupList
    // Do it
    Group group(m_root, groupId);
    if (m_session.isAdmin() || !group.unlisted().get()) {
        group.forums()
            .sort()
            .sortLexicographical()
            .by(m_root.forumRoot().subtree("*").hashKey("header").field("key"))
            .get(m_root.forumRoot().subtree("*").hashKey("header").field("newsgroup"))
            .getResult(result);
    }
}
