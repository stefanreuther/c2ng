/**
  *  \file server/common/user.cpp
  *  \brief Class server::common::User
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
#include "server/common/user.hpp"
#include "server/common/root.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "server/types.hpp"

server::common::User::User(Root& root, String_t userId)
    : m_user(root.userRoot().subtree(userId)),
      m_defaultProfile(root.defaultProfile())
{
    // ex User::User
}

// Get user's screen name.
String_t
server::common::User::getScreenName()
{
    // ex User::getScreenName
    // Since 20110901, there is no longer an automatic fallback to the login name, all users' screen names are filled in.
    // This is also the reason there is no need to implement a fallback to the default profile.
    return profile().stringField("screenname").get();
}

// Get user's login name.
String_t
server::common::User::getLoginName()
{
    // ex User::getLoginName
    return m_user.stringKey("name").get();
}

// Get user's real name.
String_t
server::common::User::getRealName()
{
    // ex User::getRealName
    std::auto_ptr<afl::data::Value> allow(getProfileRaw("inforealnameflag"));
    if (allow.get() != 0 && toInteger(allow.get()) > 0) {
        return profile().stringField("realname").get();
    } else {
        return String_t();
    }
}

// Get user's email address.
String_t
server::common::User::getEmailAddress()
{
    return profile().stringField("email").get();
}

// Get raw value from user profile.
afl::data::Value*
server::common::User::getProfileRaw(String_t key)
{
    // ex User::getProfileRaw
    afl::data::Value* result = profile().field(key).getRawValue();
    if (result == 0) {
        result = m_defaultProfile.field(key).getRawValue();
    }
    return result;
}

// Get string value from user profile.
String_t
server::common::User::getProfileString(String_t key)
{
    std::auto_ptr<afl::data::Value> p(getProfileRaw(key));
    return toString(p.get());
}

// Access user tree in database.
afl::net::redis::Subtree
server::common::User::tree()
{
    return m_user;
}

// Get user's profile.
afl::net::redis::HashKey
server::common::User::profile()
{
    // ex User::profile
    return m_user.hashKey("profile");
}

// Check existence of a user.
bool
server::common::User::exists(Root& root, String_t userId)
{
    return root.userRoot().stringSetKey("all").contains(userId);
}
