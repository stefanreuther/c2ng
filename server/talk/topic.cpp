/**
  *  \file server/talk/topic.cpp
  *  \brief Class server::talk::Topic
  */

#include "server/talk/topic.hpp"
#include "afl/data/stringlist.hpp"
#include "server/errors.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/root.hpp"
#include "server/talk/user.hpp"

// Constructor.
server::talk::Topic::Topic(Root& root, int32_t topicId)
    : m_topic(root.topicRoot().subtree(topicId)),
      m_topicId(topicId)
{
    // ex Topic::Topic
}

// Access topic header.
afl::net::redis::HashKey
server::talk::Topic::header()
{
    // ex Topic::header
    return m_topic.hashKey("header");
}

// Access topic subject.
afl::net::redis::StringField
server::talk::Topic::subject()
{
    // ex Topic::subject
    return header().stringField("subject");
}

// Access topic's forum Id.
afl::net::redis::IntegerField
server::talk::Topic::forumId()
{
    // ex Topic::forumId
    return header().intField("forum");
}

// Access topic's first posting Id.
afl::net::redis::IntegerField
server::talk::Topic::firstPostingId()
{
    // ex Topic::firstPostingId
    return header().intField("firstpost");
}

// Access topic's read permissions.
afl::net::redis::StringField
server::talk::Topic::readPermissions()
{
    // ex Topic::readPermissions
    return header().stringField("readperm");
}

// Access topic's answer permissions.
afl::net::redis::StringField
server::talk::Topic::answerPermissions()
{
    // ex Topic::answerPermissions
    return header().stringField("answerperm");
}

// Access topic's last posting Id.
afl::net::redis::IntegerField
server::talk::Topic::lastPostId()
{
    // ex Topic::lastPost
    return header().intField("lastpost");
}

// Access topic's last modification time.
afl::net::redis::IntegerField
server::talk::Topic::lastTime()
{
    // ex Topic::lastTime
    return header().intField("lasttime");
}

// Access topic's messages.
afl::net::redis::IntegerSetKey
server::talk::Topic::messages()
{
    // ex Topic::messages
    return m_topic.intSetKey("messages");
}

// Access topic's watchers.
afl::net::redis::StringSetKey
server::talk::Topic::watchers()
{
    // ex Topic::watchers
    return m_topic.stringSetKey("watchers");
}

// Access topic's forum.
server::talk::Forum
server::talk::Topic::forum(Root& root)
{
    // ex Topic::forum
    return Forum(root, forumId().get());
}

// Remove this topic.
void
server::talk::Topic::remove(Root& root)
{
    // ex Topic::remove
    // This is a simple, not too efficient implementation: just remove all messages.
    // The last Message::remove() will call Topic::removeEmpty().
    // This contains a few more database lookups than necessary (e.g. the mapping from
    // the message to the topic), but then, topic removal hopefully isn't frequent.
    afl::data::IntegerList_t msg;
    messages().getAll(msg);
    for (size_t i = 0; i < msg.size(); ++i) {
        Message(root, msg[i]).remove(root);
    }
}

// Remove this empty topic.
void
server::talk::Topic::removeEmpty(Root& root)
{
    // ex Topic::removeEmpty
    Forum f(forum(root));

    // Remove from lists
    f.topics().remove(m_topicId);
    f.stickyTopics().remove(m_topicId);

    // Remove watchers
    afl::data::StringList_t w;
    watchers().getAll(w);
    for (size_t i = 0; i < w.size(); ++i) {
        User u(root, w[i]);
        u.watchedTopics().remove(m_topicId);
        u.notifiedTopics().remove(m_topicId);
    }

    // Remove topic
    header().remove();
    messages().remove();
    watchers().remove();
}

// Check existence.
bool
server::talk::Topic::exists()
{
    // ex Topic::exists
    // A topic exists if it has any header information.
    // Mandatory header information is a forum link, so a topic cannot sensibly exist without a header.
    return header().exists();
}

// Describe topic.
server::interface::TalkThread::Info
server::talk::Topic::describe()
{
    // ex Topic::describe
    // FIXME: can we use HMGET?
    server::interface::TalkThread::Info result;
    result.subject = subject().get();
    result.forumId = forumId().get();
    result.firstPostId = firstPostingId().get(); // FIXME: name clash
    result.lastPostId = lastPostId().get();
    result.lastTime = lastTime().get();
    result.isSticky = isSticky();
    return result;
}

// Check stickyness.
bool
server::talk::Topic::isSticky()
{
    // ex Topic::isSticky
    return header().intField("sticky").get();
}

// Set stickyness.
void
server::talk::Topic::setSticky(Root& root, bool enable)
{
    // ex Topic::setSticky
    bool isSticky = header().intField("sticky").get();
    if (isSticky != enable) {
        header().intField("sticky").set(enable);
        Forum f(root, forumId().get());
        if (enable) {
            // normal -> sticky
            f.topics().moveTo(m_topicId, f.stickyTopics());
        } else {
            // sticky -> normal
            f.stickyTopics().moveTo(m_topicId, f.topics());
        }
    }
}

// Get topic Id.
int32_t
server::talk::Topic::getId() const
{
    // ex Topic::getId
    return m_topicId;
}

server::talk::Topic::TopicSorter::TopicSorter(Root& root)
    : Sorter(),
      m_root(root)
{ }

void
server::talk::Topic::TopicSorter::applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const
{
    // ex ListParams::threadSortKeys
    if (keyName == "FIRSTPOST") {
        op.by(m_root.topicRoot().subtree("*").hashKey("header").field("firstpost"));
    } else if (keyName == "FORUM") {
        op.by(m_root.topicRoot().subtree("*").hashKey("header").field("forum"));
    } else if (keyName == "LASTPOST") {
        op.by(m_root.topicRoot().subtree("*").hashKey("header").field("lastpost"));
    } else if (keyName == "LASTTIME") {
        op.by(m_root.topicRoot().subtree("*").hashKey("header").field("lasttime"));
    } else if (keyName == "SUBJECT") {
        op.by(m_root.topicRoot().subtree("*").hashKey("header").field("lasttime")).sortLexicographical();
    } else {
        throw std::runtime_error(INVALID_SORT_KEY);
    }
}
