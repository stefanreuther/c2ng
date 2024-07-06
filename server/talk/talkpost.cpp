/**
  *  \file server/talk/talkpost.cpp
  *  \brief Class server::talk::TalkPost
  */

#include "server/talk/talkpost.hpp"
#include "server/errors.hpp"
#include "server/talk/accesschecker.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/notify.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/render.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/spam.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"

namespace {
    /** Read permissions that are assigned to topics identified as being spam. */
    const char* SPAM_READ_PERM = "p:spam";

    /** Answer permissions that are assigned to topics identified as being spam. */
    const char* SPAM_ANSWER_PERM = "p:spam";
}

// Constructor.
server::talk::TalkPost::TalkPost(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

int32_t
server::talk::TalkPost::create(int32_t forumId, String_t subject, String_t text, const CreateOptions& options)
{
    // ex planetscentral/talk/cmdpost.cc:doPostNew
    // Check user permissions and find effective user
    String_t user;
    if (m_session.isAdmin()) {
        const String_t* p = options.userId.get();
        if (p == 0) {
            throw std::runtime_error(MUST_HAVE_USER_CONTEXT); // @change was 400 Need user in PCC2
        }
        user = *p;
    } else {
        const String_t* p = options.userId.get();
        if (p != 0 && *p != m_session.getUser()) {
            throw std::runtime_error(USER_NOT_ALLOWED);
        }
        user = m_session.getUser();
    }

    // Do it
    Forum f(m_root, forumId);
    if (!f.exists(m_root)) {
        throw std::runtime_error(FORUM_NOT_FOUND);
    }
    m_session.checkPermission(f.writePermissions().get(), m_root);

    const Time_t time = m_root.getTime();
    bool isSpam = false;
    User u(m_root, user);

    // Spam check
    if (!u.isAllowedToPost()) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
    if (checkSpam(subject, text, time, u, m_root.recognizer(), m_root.log())) {
        // Mark it
        isSpam = true;
    }

    // All preconditions fulfilled, operate!
    const int32_t mid = ++m_root.lastMessageId();
    const int32_t tid = ++m_root.lastTopicId();

    Message msg(m_root, mid);
    Topic topic(m_root, tid);

    // Configure message
    msg.topicId().set(tid);
    msg.postTime().set(time);
    msg.author().set(user);
    msg.sequenceNumber().set(++f.lastMessageSequenceNumber());
    msg.text().set(text);
    msg.subject().set(subject);

    // Configure topic
    topic.subject().set(subject);
    topic.forumId().set(forumId);
    topic.firstPostingId().set(mid);
    if (isSpam) {
        topic.readPermissions().set(SPAM_READ_PERM);
        topic.answerPermissions().set(SPAM_ANSWER_PERM);
    } else {
        if (const String_t* p = options.readPermissions.get()) {
            topic.readPermissions().set(*p);
        }
        if (const String_t* p = options.answerPermissions.get()) {
            topic.answerPermissions().set(*p);
        }
    }
    topic.lastPostId().set(mid);
    topic.lastTime().set(time);

    // Update forum
    f.lastPostId().set(mid);
    f.lastTime().set(time);

    // Add message to sets. We can add to f.topics() because topics are all born unsticky.
    topic.messages().add(mid);
    f.messages().add(mid);
    f.topics().add(tid);
    u.postedMessages().add(mid);

    // Notify
    if (!isSpam) {
        notifyMessage(msg, topic, f, m_root);
    }

    // Auto-watch
    if (u.isAutoWatch()) {
        u.watchedTopics().add(tid);
        topic.watchers().add(user);
    }

    return mid;
}

int32_t
server::talk::TalkPost::reply(int32_t parentPostId, String_t subject, String_t text, const ReplyOptions& options)
{
    // ex planetscentral/talk/cmdpost.cc:doPostReply
    // Check user permissions and find effective user
    String_t user;
    if (m_session.isAdmin()) {
        const String_t* p = options.userId.get();
        if (p == 0) {
            throw std::runtime_error(MUST_HAVE_USER_CONTEXT); // @change was 400 Need user in PCC2
        }
        user = *p;
    } else {
        const String_t* p = options.userId.get();
        if (p != 0 && *p != m_session.getUser()) {
            throw std::runtime_error(USER_NOT_ALLOWED);
        }
        user = m_session.getUser();
    }

    // Do it
    Message parent(m_root, parentPostId);
    if (!parent.exists()) {
        throw std::runtime_error(MESSAGE_NOT_FOUND);
    }

    const int32_t tid = parent.topicId().get();
    Topic topic(m_root, tid);

    const int32_t fid = topic.forumId().get();
    Forum f(m_root, fid);

    String_t answerperm = topic.answerPermissions().get();
    if (answerperm.empty()) {
        answerperm = f.answerPermissions().get();
    }
    if (answerperm.empty()) {
        answerperm = f.writePermissions().get();
    }
    m_session.checkPermission(answerperm, m_root);

    // Permission check
    User u(m_root, user);
    if (!u.isAllowedToPost()) {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    // All preconditions fulfilled, operate!
    const int32_t mid = ++m_root.lastMessageId();
    const int32_t time = m_root.getTime();
    Message msg(m_root, mid);

    // Configure message
    msg.topicId().set(tid);
    msg.parentMessageId().set(parentPostId);
    msg.postTime().set(time);
    msg.author().set(user);
    if (subject.empty()) {
        subject = parent.subject().get();
    }
    msg.subject().set(subject);
    msg.sequenceNumber().set(++f.lastMessageSequenceNumber());
    msg.text().set(text);

    // Update topic
    topic.lastPostId().set(mid);
    topic.lastTime().set(time);

    // Update forum
    f.lastPostId().set(mid);
    f.lastTime().set(time);

    // Add message to sets
    topic.messages().add(mid);
    f.messages().add(mid);
    u.postedMessages().add(mid);

    // Notify
    notifyMessage(msg, topic, f, m_root);

    // Auto-watch
    if (u.isAutoWatch()) {
        u.watchedTopics().add(tid);
        topic.watchers().add(user);
    }

    return mid;
}

void
server::talk::TalkPost::edit(int32_t postId, String_t subject, String_t text)
{
    // ex planetscentral/talk/cmdpost.cc:doPostEdit
    Message msg(m_root, postId);
    if (!msg.exists()) {
        throw std::runtime_error(MESSAGE_NOT_FOUND);
    }
    if (!m_session.isAdmin() && msg.author().get() != m_session.getUser()) {
        throw std::runtime_error(NOT_AUTHOR);
    }
    if (msg.subject().get() == subject && msg.text().get() == text) {
        // FIXME: was 'conn.sendSuccess("OK, unchanged");'
        return;
    }

    // Update message
    const int32_t time = m_root.getTime();
    msg.subject().set(subject);
    msg.text().set(text);
    msg.editTime().set(time);

    // Update topic
    Topic topic(m_root, msg.topicId().get());
    topic.lastTime().set(time);
    if (postId == topic.firstPostingId().get()) {
        topic.subject().set(subject);
    }

    // Update forum
    Forum f(m_root, topic.forumId().get());
    f.lastTime().set(time);
    String_t rfcMsgId = msg.rfcMessageId().get();
    msg.previousSequenceNumber().set(msg.sequenceNumber().get());
    msg.previousRfcMessageId().set(rfcMsgId);
    msg.removeRfcMessageId(m_root, rfcMsgId);
    msg.sequenceNumber().set(++f.lastMessageSequenceNumber());
    msg.rfcMessageId().remove();
    msg.rfcHeaders().remove();
}

String_t
server::talk::TalkPost::render(int32_t postId, const server::interface::TalkRender::Options& options)
{
    // ex planetscentral/talk/cmdpost.cc:doPostRender
    Message msg(m_root, postId);
    if (!msg.exists()) {
        throw std::runtime_error(MESSAGE_NOT_FOUND);
    }
    AccessChecker(m_root, m_session).checkMessage(msg);

    render::Context ctx(m_session.getUser());
    ctx.setMessageId(postId);

    render::Options temporaryOptions(m_session.renderOptions());
    temporaryOptions.updateFrom(options);

    return render::renderText(msg.text().get(), ctx, temporaryOptions, m_root);
}

void
server::talk::TalkPost::render(afl::base::Memory<const int32_t> postIds, afl::data::StringList_t& result)
{
    // ex planetscentral/talk/cmdpost.cc:doPostMRender
    AccessChecker checker(m_root, m_session);
    render::Context ctx(m_session.getUser());
    while (const int32_t* p = postIds.eat()) {
        Message msg(m_root, *p);
        if (!msg.exists() || !checker.isAllowed(msg)) {
            result.push_back("");
        } else {
            ctx.setMessageId(*p);
            result.push_back(render::renderText(msg.text().get(), ctx, m_session.renderOptions(), m_root));
        }
    }
}

server::interface::TalkPost::Info
server::talk::TalkPost::getInfo(int32_t postId)
{
    // ex planetscentral/talk/cmdpost.cc:doPostStat
    Message msg(m_root, postId);
    if (!msg.exists()) {
        throw std::runtime_error(MESSAGE_NOT_FOUND);
    }
    AccessChecker(m_root, m_session).checkMessage(msg);
    return msg.describe(m_root);
}

void
server::talk::TalkPost::getInfo(afl::base::Memory<const int32_t> postIds, afl::container::PtrVector<Info>& result)
{
    // ex planetscentral/talk/cmdpost.cc:doPostMStat
    AccessChecker checker(m_root, m_session);
    render::Context ctx(m_session.getUser());
    while (const int32_t* p = postIds.eat()) {
        Message msg(m_root, *p);
        if (!msg.exists() || !checker.isAllowed(msg)) {
            result.pushBackNew(0);
        } else {
            result.pushBackNew(new Info(msg.describe(m_root)));
        }
    }
}

String_t
server::talk::TalkPost::getHeaderField(int32_t postId, String_t fieldName)
{
    // ex planetscentral/talk/cmdpost.cc:doPostGet
    Message msg(m_root, postId);
    if (!msg.exists()) {
        throw std::runtime_error(MESSAGE_NOT_FOUND);
    }
    AccessChecker(m_root, m_session).checkMessage(msg);
    if (fieldName == "rfcmsgid") {
        // FIXME: describe() uses "msgid"
        return msg.getRfcMessageId(m_root);
    } else {
        return msg.header().stringField(fieldName).get();
    }
}

bool
server::talk::TalkPost::remove(int32_t postId)
{
    // ex planetscentral/talk/cmdpost.cc:doPostRemove
    Message msg(m_root, postId);
    if (msg.exists()) {
        // Permission check
        if (!m_session.isAdmin()
            && m_session.getUser() != msg.author().get()
            && !m_session.hasPermission(msg.topic(m_root).forum(m_root).deletePermissions().get(), m_root))
        {
            throw std::runtime_error(NOT_AUTHOR);
        }

        // Do it
        msg.remove(m_root);
        return 1;
    } else {
        return 0;
    }
}

void
server::talk::TalkPost::getNewest(int count, afl::data::IntegerList_t& postIds)
{
    // ex planetscentral/talk/cmdpost.cc:doPostListNew
    // Do not check more than this number of postings.
    // This is to avoid checking the whole database for a user who cannot see anything.
    const int32_t MAX_POSTS_TO_CHECK = 200;

    // Do it
    int32_t mid = m_root.lastMessageId().get();
    int32_t did = 0;
    AccessChecker checker(m_root, m_session);
    while (mid > 0 && did < MAX_POSTS_TO_CHECK && int(postIds.size()) < count) {
        // Check this one
        Message m(m_root, mid);
        if (m.exists() && checker.isAllowed(m)) {
            postIds.push_back(mid);
        }

        // Advance
        --mid;
        ++did;
    }
}
