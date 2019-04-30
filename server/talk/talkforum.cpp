/**
  *  \file server/talk/talkforum.cpp
  *  \brief Class server::talk::TalkForum
  */

#include <stdexcept>
#include "server/talk/talkforum.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/message.hpp"
#include "server/errors.hpp"

namespace {
    void configureForum(server::talk::Forum& f, server::talk::Root& root, afl::base::Memory<const String_t> config)
    {
        while (const String_t* pKey = config.eat()) {
            const String_t* pValue = config.eat();
            if (pValue == 0) {
                throw std::runtime_error(server::INVALID_NUMBER_OF_ARGUMENTS);
            }
            if (*pKey == "parent") {
                f.setParent(*pValue, root);
            } else if (*pKey == "newsgroup") {
                f.setNewsgroup(*pValue, root);
            } else {
                f.header().stringField(*pKey).set(*pValue);
            }
        }
    }
}

server::talk::TalkForum::TalkForum(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

int32_t
server::talk::TalkForum::add(afl::base::Memory<const String_t> config)
{
    // ex doForumAdd
    // Allocate FID
    m_session.checkAdmin();
    int32_t newFid = ++m_root.lastForumId();

    // Create forum
    Forum f(m_root, newFid);
    m_root.allForums().add(newFid);
    f.creationTime().set(m_root.getTime());

    // Configure it
    configureForum(f, m_root, config);

    // Result
    return newFid;
}

void
server::talk::TalkForum::configure(int32_t fid, afl::base::Memory<const String_t> config)
{
    // ex doForumSet
    m_session.checkAdmin();
    Forum f(m_root, fid);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }

    configureForum(f, m_root, config);
}

afl::data::Value*
server::talk::TalkForum::getValue(int32_t fid, String_t keyName)
{
    // ex doForumGet
    Forum f(m_root, fid);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }
    return f.header().field(keyName).getRawValue();
}

server::interface::TalkForum::Info
server::talk::TalkForum::getInfo(int32_t fid)
{
    // ex doForumStat
    Forum f(m_root, fid);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }
    return f.describe(server::talk::render::Context(m_session.getUser()), m_session.renderOptions(), m_root);
}

void
server::talk::TalkForum::getInfo(afl::base::Memory<const int32_t> fids, afl::container::PtrVector<Info>& result)
{
    // ex doForumMStat
    server::talk::render::Context ctx(m_session.getUser());
    while (const int32_t* fid = fids.eat()) {
        Forum f(m_root, *fid);
        if (!f.exists(m_root)) {
            // FIXME: this is consistent with PCC2 c2talk - but should we return null instead?
            throw std::runtime_error(FORUM_NOT_FOUND);
        }
        result.pushBackNew(new Info(f.describe(ctx, m_session.renderOptions(), m_root)));
    }
}

int32_t
server::talk::TalkForum::getPermissions(int32_t fid, afl::base::Memory<const String_t> permissionList)
{
    // ex doForumPerms
    int32_t result = 0;
    int32_t mask = 1;

    // Do it
    Forum f(m_root, fid);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }

    while (const String_t* p = permissionList.eat()) {
        String_t key = *p + "perm";
        String_t str = f.header().stringField(key).get();
        if (m_session.hasPermission(str, m_root)) {
            result |= mask;
        }
        mask <<= 1;
    }
    return result;
}

server::interface::TalkForum::Size
server::talk::TalkForum::getSize(int32_t fid)
{
    // ex doForumSize
    Forum f(m_root, fid);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }
    m_session.checkPermission(f.readPermissions().get(), m_root);

    Size sz;
    sz.numThreads       = f.topics().size();
    sz.numStickyThreads = f.stickyTopics().size();
    sz.numMessages      = f.messages().size();
    return sz;
}

afl::data::Value*
server::talk::TalkForum::getThreads(int32_t fid, const ListParameters& params)
{
    // ex doForumListThreads
    Forum f(m_root, fid);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }

    return executeListOperation(params, f.topics(), Topic::TopicSorter(m_root));
}

afl::data::Value*
server::talk::TalkForum::getStickyThreads(int32_t fid, const ListParameters& params)
{
    // ex doForumListStickyThreads
    Forum f(m_root, fid);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }

    return executeListOperation(params, f.stickyTopics(), Topic::TopicSorter(m_root));
}

afl::data::Value*
server::talk::TalkForum::getPosts(int32_t fid, const ListParameters& params)
{
    // ex doForumListPosts
    Forum f(m_root, fid);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }

    return executeListOperation(params, f.messages(), Message::MessageSorter(m_root));
}

int32_t
server::talk::TalkForum::findForum(String_t key)
{
    return m_root.forumMap().intField(key).get();
}

afl::data::Value*
server::talk::TalkForum::executeListOperation(const ListParameters& params, afl::net::redis::IntegerSetKey key, const Sorter& sorter)
{
    // ex ListParams::exec
    switch (params.mode) {
     case ListParameters::WantAll:
     case ListParameters::WantRange: {
        afl::net::redis::SortOperation op(key.sort());
        if (params.mode == ListParameters::WantRange) {
            op.limit(params.start, params.count);
        }
        if (const String_t* sortKey = params.sortKey.get()) {
            sorter.applySortKey(op, *sortKey);
        }
        return op.getResult();
     }
     case ListParameters::WantMemberCheck:
        return makeIntegerValue(key.contains(params.item));

     case ListParameters::WantSize:
        return makeIntegerValue(key.size());
    }
    return 0;
}
