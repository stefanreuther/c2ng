/**
  *  \file server/common/user.hpp
  *  \brief Class server::common::User
  */
#ifndef C2NG_SERVER_COMMON_USER_HPP
#define C2NG_SERVER_COMMON_USER_HPP

#include "afl/net/redis/subtree.hpp"
#include "afl/net/redis/hashkey.hpp"

namespace server { namespace common {

    class Root;

    /** A user profile.
        This encapsulates the user profile access basics. */
    class User {
     public:
        /** Constructor.
            \param root Service root
            \param userId User Id ("1001") */
        User(Root& root, String_t userId);

        /** Get user Id.
            \return user Id */
        const String_t& getUserId() const;

        /** Get user's screen name.
            This value is stored in the user's profile.
            \return Screen Name */
        String_t getScreenName();

        /** Get user's login name.
            This is the name he uses to login, and which others use to refer to him.
            \return Login Name */
        String_t getLoginName();

        /** Get user's real name.
            If this user's real name is not available to others (configuration option), returns a null string.
            \return Real Name or empty string */
        String_t getRealName();

        /** Get user's email address.
            \return Email address or empty string */
        String_t getEmailAddress();

        /** Get raw value from user profile.
            If the value is not set, falls back to the default from the default profile.
            \param key Profile key
            \return Option value. Null if not set anywhere */
        afl::data::Value* getProfileRaw(String_t key);

        /** Get string value from user profile.
            If the value is not set, falls back to the default from the default profile.
            \param key Profile key
            \return Option value. Empty if not set anywhere */
        String_t getProfileString(String_t key);

        /** Access user tree in database.
            \return database subtree */
        afl::net::redis::Subtree tree();

        /** Get user's profile.
            Used for manual access to user's configuration (not recommended normally).
            \return profile hash */
        afl::net::redis::HashKey profile();

        /** Check existence of a user.
            \param root Service root
            \param userId User Id ("1001") */
        static bool exists(Root& root, String_t userId);

     private:
        const std::string m_userId;
        afl::net::redis::Subtree m_user;
        afl::net::redis::HashKey m_defaultProfile;
    };

} }

#endif
