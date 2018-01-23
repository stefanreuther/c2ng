/**
  *  \file server/common/root.cpp
  *  \brief Class server::common::Root
  */

#include "server/common/root.hpp"

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
