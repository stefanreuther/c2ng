/**
  *  \file server/user/user.hpp
  */
#ifndef C2NG_SERVER_USER_USER_HPP
#define C2NG_SERVER_USER_USER_HPP

#include "server/common/user.hpp"
#include "afl/net/redis/stringsetkey.hpp"

namespace server { namespace user {

    class Root;

    /** A user profile.
        This encapsulates the user profile access for c2user.
        It is based on the common User class. */
    class User : public server::common::User {
     public:
        /** Constructor.
            \param root Service root
            \param userId User Id ("1001") */
        User(Root& root, String_t userId);

        /** Get user's password hash.
            \return password hash */
        afl::net::redis::StringKey passwordHash();

        /** Get set of tokens by type.
            \param type Type */
        afl::net::redis::StringSetKey tokensByType(String_t type);

        /** Access user data.
            \return user data subtree */
        afl::net::redis::Subtree userData();
    };

} }

#endif
