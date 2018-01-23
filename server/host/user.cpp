/**
  *  \file server/host/user.cpp
  *  \brief Class server::host::User
  */

#include "server/host/user.hpp"
#include "server/host/root.hpp"
#include "afl/string/format.hpp"

// Constructor.
server::host::User::User(Root& root, String_t userId)
    : server::common::User(root, userId)
{
    // ex User::User
}

// Destructor.
server::host::User::~User()
{ }

// Access set of owned games.
afl::net::redis::IntegerSetKey
server::host::User::ownedGames()
{
    return tree().intSetKey("ownedGames");
}

// Access user history.
afl::net::redis::StringListKey
server::host::User::history()
{
    return tree().stringListKey("history");
}

// Access game reference counts.
afl::net::redis::HashKey
server::host::User::gameReferenceCounts()
{
    return tree().hashKey("games");
}

// Access game reference count for one game.
afl::net::redis::IntegerField
server::host::User::gameReferenceCount(int32_t gameId)
{
    return gameReferenceCounts().intField(afl::string::Format("%d", gameId));
}
