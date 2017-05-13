/**
  *  \file server/talk/user.hpp
  *  \brief Class server::talk::User
  */
#ifndef C2NG_SERVER_TALK_USER_HPP
#define C2NG_SERVER_TALK_USER_HPP

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerkey.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/string.hpp"

namespace server { namespace talk {

    class Root;

    /** A user profile.
        This encapsulates the user profile access for c2talk.

        \todo Merge with User classes of other components */
    class User {
     public:
        /** Constructor.
            \param root Service root
            \param userId User Id ("1001") */
        User(Root& root, String_t userId);

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

        /** Get raw value from user profile.
            If the value is not set, falls back to the default from the default profile.
            \param key Profile key
            \return Option value. Null if not set anywhere */
        afl::data::Value* getProfileRaw(String_t key);

        /** Get PM mail type (profile access).
            \return Value from profile */
        String_t getPMMailType();

        /** Get autowatch flag (profile access).
            If set, the user automatically watches topic he posts in.
            \return Value */
        bool isAutoWatch();

        /** Get watch-individual flag (profile access).
            If set, the user wants notifications about each message for watched topics/forums.
            \return Value */
        bool isWatchIndividual();

        /** Get user's profile.
            Used for manual access to user's configuration (not recommended normally).
            \return profile hash */
        afl::net::redis::HashKey profile();

        /** Get forum data for user.
            Used for manual access to user's forum data (not recommended normally).
            \return subtree containing forum data. */
        afl::net::redis::Subtree forumData();

        /** Get set of user's posted messages.
            If a Message's Message::author() points at this user, the Message::getId() should be stored here.
            \return IntegerSetKey containing Message Ids */
        afl::net::redis::IntegerSetKey postedMessages();

        /** Get newsrc data for user.
            \return subtree for newsrc data */
        afl::net::redis::Subtree newsrc();

        /** Get user's password hash.
            \return password hash */
        afl::net::redis::StringKey passwordHash();

        /** Get PM data for user.
            Used for manual access to user's PM data (not recommended normally).
            \return subtree containing forum data. */
        afl::net::redis::Subtree pmFolderData();

        /** Get user's PM folder counter.
            This is the last assigned PM folder Id.
            \return folder counter */
        afl::net::redis::IntegerKey pmFolderCount();

        /** Get user's PM folders.
            \return set of all folders */
        afl::net::redis::IntegerSetKey pmFolders();

        /** Get list of forums watched by user.
            \return Forum Ids of watched forums */
        afl::net::redis::IntegerSetKey watchedForums();

        /** Get list of topics watched by user.
            \return Topic Ids of watched topics */
        afl::net::redis::IntegerSetKey watchedTopics();

        /** Get list of notified forums.
            If a user requests to be notified once instead of for each single message,
            a forum is marked notified when a notification has been sent until they visit the forum again.
            \return Forum Ids of notified forums */
        afl::net::redis::IntegerSetKey notifiedForums();

        /** Get list of notified topics.
            If a user requests to be notified once instead of for each single message,
            a topic is marked notified when a notification has been sent until they visit the topic again.
            \return Forum Ids of notified forums */
        afl::net::redis::IntegerSetKey notifiedTopics();

     private:
        afl::net::redis::Subtree m_user;
        afl::net::redis::HashKey m_defaultProfile;
    };

} }

#endif
