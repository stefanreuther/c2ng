/**
  *  \file server/talk/forum.cpp
  *  \brief Class server::talk::Forum
  */

#include "server/talk/forum.hpp"
#include "server/errors.hpp"
#include "server/talk/group.hpp"
#include "server/talk/render/render.hpp"
#include "server/talk/session.hpp"

// Constructor.
server::talk::Forum::Forum(Root& root, int32_t forumId)
    : m_forum(root.forumRoot().subtree(forumId)),
      m_forumId(forumId)
{
    // ex Forum::Forum
}

// Access header.
afl::net::redis::HashKey
server::talk::Forum::header()
{
    // ex Forum::header
    return m_forum.hashKey("header");
}

// Access forum name.
afl::net::redis::StringField
server::talk::Forum::name()
{
    // ex Forum::name
    return header().stringField("name");
}

// Access forum description.
afl::net::redis::StringField
server::talk::Forum::description()
{
    // ex Forum::description
    return header().stringField("description");
}

// Access read permissions.
afl::net::redis::StringField
server::talk::Forum::readPermissions()
{
    // ex Forum::readPermissions
    return header().stringField("readperm");
}

// Access write permissions.
afl::net::redis::StringField
server::talk::Forum::writePermissions()
{
    // ex Forum::writePermissions
    return header().stringField("writeperm");
}

// Access answer permissions.
afl::net::redis::StringField
server::talk::Forum::answerPermissions()
{
    // ex Forum::answerPermissions
    return header().stringField("answerperm");
}

// Access delete permissions.
afl::net::redis::StringField
server::talk::Forum::deletePermissions()
{
    // ex Forum::deletePermissions
    return header().stringField("deleteperm");
}

// Access sort key.
afl::net::redis::StringField
server::talk::Forum::key()
{
    // ex Forum::key
    return header().stringField("key");
}

// Access last message sequence number.
afl::net::redis::IntegerField
server::talk::Forum::lastMessageSequenceNumber()
{
    // ex Forum::lastMessageSequenceNumber
    return header().intField("msgseq");
}

// Access creation time.
afl::net::redis::IntegerField
server::talk::Forum::creationTime()
{
    // ex Forum::creationTime
    return header().intField("time");
}

// Access last post Id.
afl::net::redis::IntegerField
server::talk::Forum::lastPostId()
{
    // ex Forum::lastPost
    return header().intField("lastpost");
}

// Access last forum modification time.
afl::net::redis::IntegerField
server::talk::Forum::lastTime()
{
    // ex Forum::lastTime
    return header().intField("lasttime");
}

// Access messages.
afl::net::redis::IntegerSetKey
server::talk::Forum::messages()
{
    // ex Forum::messages
    return m_forum.intSetKey("messages");
}

// Access topics.
afl::net::redis::IntegerSetKey
server::talk::Forum::topics()
{
    // ex Forum::topics
    return m_forum.intSetKey("threads");
}

// Access sticky topics.
afl::net::redis::IntegerSetKey
server::talk::Forum::stickyTopics()
{
    // ex Forum::stickyTopics
    return m_forum.intSetKey("stickythreads");
}

// Access watchers.
afl::net::redis::StringSetKey
server::talk::Forum::watchers()
{
    // ex Forum::watchers
    return m_forum.stringSetKey("watchers");
}

// Set parent group.
void
server::talk::Forum::setParent(String_t newParent, Root& root)
{
    // ex Forum::setParent
    String_t oldParent = getParent();
    if (oldParent != newParent) {
        Group oldGroup(root, oldParent);
        Group newGroup(root, newParent);
        if (oldParent.empty()) {
            newGroup.forums().add(m_forumId);
        } else if (newParent.empty()) {
            oldGroup.forums().remove(m_forumId);
        } else {
            oldGroup.forums().moveTo(m_forumId, newGroup.forums());
        }
        header().stringField("parent").set(newParent);
    }
}

// Get parent group.
String_t
server::talk::Forum::getParent()
{
    // ex Forum::getParent
    return header().stringField("parent").get();
}

// Set newsgroup name.
void
server::talk::Forum::setNewsgroup(String_t newNG, Root& root)
{
    // ex Forum::setNewsgroup
    String_t oldNG(getNewsgroup());
    afl::net::redis::HashKey ngMap(root.newsgroupMap());

    if (oldNG != newNG) {
        // If this newsgroup name is already taken, drop it from there
        int32_t oldForum(ngMap.intField(newNG).get());
        if (oldForum != 0) {
            Forum(root, oldForum).header().field("newsgroup").remove();
        }

        // Also remove our own old name
        if (!oldNG.empty()) {
            ngMap.field(oldNG).remove();
        }

        // Update
        ngMap.intField(newNG).set(m_forumId);
        header().stringField("newsgroup").set(newNG);
    }
}

// Get newsgroup name.
String_t
server::talk::Forum::getNewsgroup()
{
    // ex Forum::getNewsgroup
    return header().stringField("newsgroup").get();
}

// Get forum Id.
int32_t
server::talk::Forum::getId() const
{
    // ex Forum::getId
    return m_forumId;
}

// Check existence.
bool
server::talk::Forum::exists(Root& root)
{
    // ex Forum::exists
    return root.allForums().contains(m_forumId);
}

server::interface::TalkForum::Info
server::talk::Forum::describe(const server::talk::render::Context& ctx, const server::talk::render::Options& opts, Root& root)
{
    // ex Forum::describe
    // FIXME: can we use HMGET?
    server::interface::TalkForum::Info result;
    result.name = name().get();
    result.parentGroup = getParent();
    result.description = server::talk::render::render(description().get(), ctx, opts, root);
    result.newsgroupName = getNewsgroup();
    return result;
}

server::interface::TalkNNTP::Info
server::talk::Forum::describeAsNewsgroup(const server::talk::render::Context& ctx, const server::talk::render::Options& opts, Root& root, Session& session)
{
    // ex Forum::describeAsNewsgroup
    server::interface::TalkNNTP::Info result;
    result.forumId = m_forumId;
    result.newsgroupName = getNewsgroup();
    result.firstSequenceNumber = 1;
    result.lastSequenceNumber = lastMessageSequenceNumber().get();
    result.writeAllowed = session.hasPermission(writePermissions().get(), root);
    result.description = server::talk::render::render(description().get(), ctx, opts, root);
    return result;
}


server::talk::Forum::ForumSorter::ForumSorter(Root& root)
    : Sorter(),
      m_root(root)
{ }

void
server::talk::Forum::ForumSorter::applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const
{
    // ex ListParams::threadSortKeys
    if (keyName == "KEY") {
        op.by(m_root.forumRoot().subtree("*").hashKey("header").field("key")).sortLexicographical();
    } else if (keyName == "LASTPOST") {
        op.by(m_root.forumRoot().subtree("*").hashKey("header").field("lastpost"));
    } else if (keyName == "LASTTIME") {
        op.by(m_root.forumRoot().subtree("*").hashKey("header").field("lasttime"));
    } else if (keyName == "NAME") {
        op.by(m_root.forumRoot().subtree("*").hashKey("header").field("name")).sortLexicographical();
    } else {
        throw std::runtime_error(INVALID_SORT_KEY);
    }
}
