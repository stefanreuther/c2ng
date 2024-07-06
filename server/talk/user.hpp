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
#include "server/common/user.hpp"

namespace server { namespace talk {

    class Root;

    /** A user profile.
        This encapsulates the user profile access for c2talk.
        It is based on the common User class. */
    class User : public server::common::User {
     public:
        /** Constructor.
            \param root Service root
            \param userId User Id ("1001") */
        User(Root& root, String_t userId);

        /** Get PM mail type (profile access).
            \return Value from profile */
        String_t getPMMailType();

        /** Get PM permission (profile access).
            If set, the user is allowed to send PMs.
            \return Value */
        bool isAllowedToSendPMs();

        /** Get post permission (profile access).
            If set, the user is allowed to post to forums.
            \return Value */
        bool isAllowedToPost();

        /** Get autowatch flag (profile access).
            If set, the user automatically watches topic he posts in.
            \return Value */
        bool isAutoWatch();

        /** Get watch-individual flag (profile access).
            If set, the user wants notifications about each message for watched topics/forums.
            \return Value */
        bool isWatchIndividual();

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
    };

} }

#endif
