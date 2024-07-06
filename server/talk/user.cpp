/**
  *  \file server/talk/user.cpp
  *  \brief Class server::talk::User
  */

#include <memory>
#include "server/talk/user.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "server/talk/root.hpp"
#include "server/types.hpp"

// Constructor.
server::talk::User::User(Root& root, String_t userId)
    : server::common::User(root, userId)
{
    // ex User::User
}

// Get PM mail type (profile access).
String_t
server::talk::User::getPMMailType()
{
    // ex User::getPMMailType
    std::auto_ptr<afl::data::Value> value(getProfileRaw("mailpmtype"));
    return toString(value.get());
}

// Get PM permission (profile access).
bool
server::talk::User::isAllowedToSendPMs()
{
    // Unset means yes
    std::auto_ptr<afl::data::Value> value(getProfileRaw("allowpm"));
    return value.get() == 0 || toInteger(value.get()) != 0;
}


// Get forum permission (profile access).
bool
server::talk::User::isAllowedToPost()
{
    // Unset means yes
    std::auto_ptr<afl::data::Value> value(getProfileRaw("allowpost"));
    return value.get() == 0 || toInteger(value.get()) != 0;
}

// Get autowatch flag (profile access).
bool
server::talk::User::isAutoWatch()
{
    // ex User::isAutoWatch
    // Unset means yes
    std::auto_ptr<afl::data::Value> value(getProfileRaw("talkautowatch"));
    return value.get() == 0 || toInteger(value.get()) != 0;
}

// Get watch-individual flag (profile access).
bool
server::talk::User::isWatchIndividual()
{
    // ex User::isWatchIndividual
    // Unset means no
    std::auto_ptr<afl::data::Value> value(getProfileRaw("talkwatchindividual"));
    return toInteger(value.get()) > 0;
}

// Get forum data for user.
afl::net::redis::Subtree
server::talk::User::forumData()
{
    // ex User::forumData
    return tree().subtree("forum");
}

// Get set of user's posted messages.
afl::net::redis::IntegerSetKey
server::talk::User::postedMessages()
{
    // ex User::postedMessages
    return forumData().intSetKey("posted");
}

// Get newsrc data for user.
afl::net::redis::Subtree
server::talk::User::newsrc()
{
    // ex User::newsrc
    return forumData().subtree("newsrc");
}

// Get PM data for user.
afl::net::redis::Subtree
server::talk::User::pmFolderData()
{
    // ex User::pmFolderData
    return tree().subtree("pm:folder");
}

// Get user's PM folder counter.
afl::net::redis::IntegerKey
server::talk::User::pmFolderCount()
{
    // ex User::pmFolderCount
    return pmFolderData().intKey("id");
}

// Get user's PM folders.
afl::net::redis::IntegerSetKey
server::talk::User::pmFolders()
{
    // ex User::pmFolders
    return pmFolderData().intSetKey("all");
}

// Get list of forums watched by user.
afl::net::redis::IntegerSetKey
server::talk::User::watchedForums()
{
    // ex User::watchedForums
    return forumData().intSetKey("watchedForums");
}

// Get list of topics watched by user.
afl::net::redis::IntegerSetKey
server::talk::User::watchedTopics()
{
    // ex User::watchedTopics
    return forumData().intSetKey("watchedThreads");
}

// Get list of notified forums.
afl::net::redis::IntegerSetKey
server::talk::User::notifiedForums()
{
    // ex User::notifiedForums
    return forumData().intSetKey("notifiedForums");
}

// Get list of notified topics.
afl::net::redis::IntegerSetKey
server::talk::User::notifiedTopics()
{
    // ex User::notifiedTopics
    return forumData().intSetKey("notifiedThreads");
}

// Rate limiting: score.
afl::net::redis::IntegerField
server::talk::User::rateScore()
{
    return profile().intField("talkratescore");
}

// Rate limiting: time.
afl::net::redis::IntegerField
server::talk::User::rateTime()
{
    return profile().intField("talkratetime");
}
