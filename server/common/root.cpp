/**
  *  \file server/common/root.cpp
  *  \brief Class server::common::Root
  */

#include "server/common/root.hpp"
#include "server/common/util.hpp"

namespace {
    /* Database root nodes.
       This is the only place containing DB root nodes.
       All other modules only use nodes derived from (and therefore below) those,
       or defined by their own Root class. */
    const char USER_ROOT[] =           "user:";
    const char DEFAULT_PROFILE[] =     "default:profile";
    const char GAME_ROOT[] =           "game:";
}

// Constructor.
server::common::Root::Root(afl::net::CommandHandler& db)
    : m_db(db)
{ }

// Access root of user database.
afl::net::redis::Subtree
server::common::Root::userRoot()
{
    // ex USER_ROOT
    return afl::net::redis::Subtree(m_db, USER_ROOT);
}

// Access root of game database.
afl::net::redis::Subtree
server::common::Root::gameRoot()
{
    return afl::net::redis::Subtree(m_db, GAME_ROOT);
}

// Access default user profile.
afl::net::redis::HashKey
server::common::Root::defaultProfile()
{
    return afl::net::redis::HashKey(m_db, DEFAULT_PROFILE);
}

// Map login name to user Id.
String_t
server::common::Root::getUserIdFromLogin(String_t login)
{
    // ex User::getUserIdFromLogin
    String_t simplifiedLogin = simplifyUserName(login);
    if (simplifiedLogin.empty()) {
        // Name consists of illegal characters only
        return String_t();
    }
    String_t userId = userByName(simplifiedLogin).get();
    if (userId.find_first_not_of("0") == String_t::npos) {
        // does not exist, return empty string
        return String_t();
    }
    return userId;
}

// Access user-by-name field.
afl::net::redis::StringKey
server::common::Root::userByName(String_t simplifiedName)
{
    return afl::net::redis::Subtree(m_db, "uid:").stringKey(simplifiedName);
}
