/**
  *  \file server/talk/talkthread.cpp
  *  \brief Class server::talk::TalkThread
  */

#include <stdexcept>
#include "server/talk/talkthread.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/accesschecker.hpp"
#include "server/talk/talkforum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/forum.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "afl/net/redis/integersetoperation.hpp"
#include "server/errors.hpp"

server::talk::TalkThread::TalkThread(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

server::talk::TalkThread::Info
server::talk::TalkThread::getInfo(int32_t threadId)
{
    // ex doThreadStat
    Topic t(m_root, threadId);
    if (!t.exists()) {
        throw std::runtime_error(TOPIC_NOT_FOUND);
    }

    AccessChecker(m_root, m_session).checkTopic(t);
    return t.describe();
}

void
server::talk::TalkThread::getInfo(afl::base::Memory<const int32_t> threadIds, afl::container::PtrVector<Info>& result)
{
    // ex doThreadMStat
    AccessChecker checker(m_root, m_session);
    while (const int32_t* p = threadIds.eat()) {
        Topic t(m_root, *p);
        if (!t.exists() || !checker.isAllowed(t)) {
            result.pushBackNew(0);
        } else {
            result.pushBackNew(new Info(t.describe()));
        }
    }
}

afl::data::Value*
server::talk::TalkThread::getPosts(int32_t threadId, const ListParameters& params)
{
    // ex doThreadListPosts
    Topic t(m_root, threadId);
    if (!t.exists()) {
        throw std::runtime_error(TOPIC_NOT_FOUND);
    }

    return TalkForum::executeListOperation(params, t.messages(), Message::MessageSorter(m_root));
}

void
server::talk::TalkThread::setSticky(int32_t threadId, bool flag)
{
    // ex doThreadSticky
    Topic t(m_root, threadId);
    if (!t.exists()) {
        throw std::runtime_error(TOPIC_NOT_FOUND);
    }

    // Permission check
    m_session.checkPermission(t.forum(m_root).deletePermissions().get(), m_root);

    // Execute
    t.setSticky(m_root, flag);
}

int
server::talk::TalkThread::getPermissions(int32_t threadId, afl::base::Memory<const String_t> permissionList)
{
    // ex doThreadPermissions
    int32_t result = 0;
    int32_t mask = 1;

    // Do it
    Topic t(m_root, threadId);
    if (!t.exists()) {
        throw std::runtime_error(TOPIC_NOT_FOUND);
    }

    Forum f = t.forum(m_root);
    while (const String_t* p = permissionList.eat()) {
        String_t key = *p + "perm";
        String_t str = t.header().stringField(key).get();
        if (str.empty()) {
            str = f.header().stringField(key).get();
        }
        if (m_session.hasPermission(str, m_root)) {
            result |= mask;
        }
        mask <<= 1;
    }
    return result;
}

void
server::talk::TalkThread::moveToForum(int32_t threadId, int32_t forumId)
{
    // ex doThreadMove
    // Do it
    Topic t(m_root, threadId);
    if (!t.exists()) {
        throw std::runtime_error(TOPIC_NOT_FOUND);
    }

    // Check forums
    Forum src(t.forum(m_root));
    Forum dst(m_root, forumId);
    if (src.getId() == forumId) {
        return;
    }
    if (!dst.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }

    // Check permissions: must have delete permission on source forum, and write permission on target
    if (!m_session.isAdmin()) {
        m_session.checkPermission(src.deletePermissions().get(), m_root);
        m_session.checkPermission(dst.writePermissions().get(), m_root);
    }

    // Do it. Actual forum move is trivial, but we must update all sequence numbers and change
    // all message Ids for the NNTP side.
    afl::data::IntegerList_t posts;
    {
        afl::net::redis::SortOperation op(t.messages().sort());
        Message::applySortBySequence(m_root, op);
        op.getResult(posts);
    }
    for (size_t i = 0; i < posts.size(); ++i) {
        Message msg(m_root, posts[i]);

        // NOTE: For a perfect mapping, we must actually make sure that the new sequence number does
        // not match ANY old sequence number, because otherwise the new message would have the same
        // generated message Id on the NNTP side. This could be trivially assured by choosing the
        // new one bigger than the old one, but that is not feasible (think moving a thread from a
        // 10000-post forum into a 10-post forum, leaving a 9990-post gap). Thus, we only avoid
        // clashes with the current and previous one.
        int32_t oldSeq1 = msg.sequenceNumber().get();
        int32_t oldSeq2 = msg.previousSequenceNumber().get();
        int32_t newSeq;
        do {
            newSeq = ++dst.lastMessageSequenceNumber();
        } while (newSeq == oldSeq1 || newSeq == oldSeq2);

        // Update the post's identifying information
        String_t rfcMsgId = msg.rfcMessageId().get();
        msg.removeRfcMessageId(m_root, rfcMsgId);
        msg.previousRfcMessageId().set(rfcMsgId);
        msg.previousSequenceNumber().set(oldSeq1);
        msg.sequenceNumber().set(newSeq);
        msg.rfcMessageId().remove();
    }

    // Move the postings into the new forum
    src.messages().remove(t.messages()).storeTo(src.messages());
    dst.messages().merge(t.messages()).storeTo(dst.messages());

    // Move the thread
    if (t.isSticky()) {
        src.stickyTopics().moveTo(threadId, dst.stickyTopics());
    } else {
        src.topics().moveTo(threadId, dst.topics());
    }
    t.forumId().set(forumId);
}

bool
server::talk::TalkThread::remove(int32_t threadId)
{
    // ex doThreadRemove
    Topic t(m_root, threadId);
    bool result;
    if (!t.exists()) {
        // Does not exist: assume success
        result = false;
    } else {
        // Check delete permissions
        Forum f(t.forum(m_root));
        if (!m_session.isAdmin()) {
            m_session.checkPermission(f.deletePermissions().get(), m_root);
        }

        // Do it
        t.remove(m_root);
        result = true;
    }
    return result;
}
