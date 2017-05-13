/**
  *  \file server/talk/user.cpp
  *  \brief Class server::talk::User
  *
  *  PCC2 comment:
  *
  *  Users have, for our purposes, three identifying names:
  *  - a user Id. This is used everywhere in the database.
  *    Although it is a number, all programs treat it as string.
  *    User Ids taken from the database are trusted. User Ids
  *    are unique and not recycled.
  *  - a login name. This is the name used in URLs. There is an
  *    index mapping login names to user Ids. This is also the
  *    name users use to refer to other users (in at-links, for
  *    example). Login names are unique, but can be recycled.
  *  - a screen name. This one is only displayed and never used
  *    in any machine interface; there is no index and there is
  *    no mechanism to make them unique.
  */

#include <memory>
#include "server/talk/user.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "server/talk/root.hpp"
#include "server/types.hpp"

// Constructor.
server::talk::User::User(Root& root, String_t userId)
    : m_user(root.userRoot().subtree(userId)),
      m_defaultProfile(root.defaultProfile())
{
    // ex User::User
}

// Get user's screen name.
String_t
server::talk::User::getScreenName()
{
    // ex User::getScreenName
    // Since 20110901, there is no longer an automatic fallback to the login name, all users' screen names are filled in.
    // This is also the reason there is no need to implement a fallback to the default profile.
    return profile().stringField("screenname").get();
}

// Get user's login name.
String_t
server::talk::User::getLoginName()
{
    // ex User::getLoginName
    return m_user.stringKey("name").get();
}

// Get user's real name.
String_t
server::talk::User::getRealName()
{
    // ex User::getRealName
    std::auto_ptr<afl::data::Value> allow(getProfileRaw("inforealnameflag"));
    if (allow.get() != 0 && toInteger(allow.get()) > 0) {
        return profile().stringField("realname").get();
    } else {
        return String_t();
    }
}

// Get raw value from user profile.
afl::data::Value*
server::talk::User::getProfileRaw(String_t key)
{
    // ex User::getProfileRaw
    afl::data::Value* result = profile().field(key).getRawValue();
    if (result == 0) {
        result = m_defaultProfile.field(key).getRawValue();
    }
    return result;
}

// Get PM mail type (profile access).
String_t
server::talk::User::getPMMailType()
{
    // ex User::getPMMailType
    std::auto_ptr<afl::data::Value> value(getProfileRaw("mailpmtype"));
    return toString(value.get());
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

// Get user's profile.
afl::net::redis::HashKey
server::talk::User::profile()
{
    // ex User::profile
    return m_user.hashKey("profile");
}

// Get forum data for user.
afl::net::redis::Subtree
server::talk::User::forumData()
{
    // ex User::forumData
    return m_user.subtree("forum");
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

// Get user's password hash.
afl::net::redis::StringKey
server::talk::User::passwordHash()
{
    return m_user.stringKey("password");
}

// Get PM data for user.
afl::net::redis::Subtree
server::talk::User::pmFolderData()
{
    // ex User::pmFolderData
    return m_user.subtree("pm:folder");
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

