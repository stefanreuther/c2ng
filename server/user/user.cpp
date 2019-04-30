/**
  *  \file server/user/user.cpp
  */

#include "server/user/user.hpp"
#include "server/user/root.hpp"

server::user::User::User(Root& root, String_t userId)
    : server::common::User(root, userId)
{ }

// Get user's password hash.
afl::net::redis::StringKey
server::user::User::passwordHash()
{
    return tree().stringKey("password");
}

afl::net::redis::StringSetKey
server::user::User::tokensByType(String_t type)
{
    return tree().subtree("tokens").stringSetKey(type);
}

afl::net::redis::Subtree
server::user::User::userData()
{
    return tree().subtree("app");
}
