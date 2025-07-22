/**
  *  \file server/talk/root.hpp
  *  \brief Class server::talk::Root
  */
#ifndef C2NG_SERVER_TALK_ROOT_HPP
#define C2NG_SERVER_TALK_ROOT_HPP

#include <memory>
#include "afl/net/commandhandler.hpp"
#include "afl/net/redis/integerkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/sys/log.hpp"
#include "afl/sys/mutex.hpp"
#include "server/common/root.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/inlinerecognizer.hpp"
#include "server/talk/linkformatter.hpp"
#include "server/talk/notifier.hpp"
#include "server/types.hpp"
#include "util/syntax/keywordtable.hpp"

namespace server { namespace talk {

    /** A talk server's root state.
        Contains global configuration and state objects.
        Root is shared between all connections.

        Root contains the top-level database layout rules.
        All accesses happen through subtree or other objects given out by Root.

        <b>Usage Guidelines:</b>

        Root produces links (afl::net::redis::Subtree) to parts of the database.
        Data model objects (Forum, Group, etc.) should never keep a reference to a Root.
        Instead, when a function needs to refer to data outside its object, pass it a Root reference as parameter,
        to make these outside accesses explicit. */
    class Root : public server::common::Root {
     public:
        /** Constructor.
            \param db Database connection
            \param config Configuration. Will be copied. */
        Root(afl::net::CommandHandler& db, const Configuration& config);

        /** Destructor. */
        ~Root();

        /*
         *  Nested Objects
         */

        /** Access mutex.
            Take this mutex before working on other subobjects (in particular, database).
            \return mutex */
        afl::sys::Mutex& mutex();

        /** Access logger.
            Attach a listener to receive log messages.
            \return logger */
        afl::sys::Log& log();

        /** Access keyword table.
            \return keyword table */
        util::syntax::KeywordTable& keywordTable();

        /** Access inline-markup recognizer.
            \return recognizer */
        InlineRecognizer& recognizer();

        /** Access link formatter.
            \return link formatter */
        LinkFormatter& linkFormatter();

        /** Access configuration.
            \return configuration */
        const Configuration& config() const;

        /** Get current time.
            The time is specified in minutes-since-epoch.
            \return time */
        Time_t getTime();

        /** Get notifier.
            \return notifier instance. Can be null. */
        Notifier* getNotifier();

        /** Set notifier.
            \param p Notifier. Root takes ownership. Can be null. */
        void setNewNotifier(Notifier* p);

        /*
         *  Database Layout
         */

        /** Access root of "group" tree.
            \return tree
            \see Group */
        afl::net::redis::Subtree groupRoot();

        /** Access root of "message" tree.
            \return tree
            \see Message */
        afl::net::redis::Subtree messageRoot();

        /** Access last message Id.
            Contains newest message Id and is incremented for each new message.
            \return key */
        afl::net::redis::IntegerKey lastMessageId();

        /** Access queue of forum messages to be notified.
            Contains a set of un-notified messages.
            \return key */
        afl::net::redis::IntegerSetKey messageNotificationQueue();

        /** Access root of "topic" tree.
            \return tree
            \see Topic */
        afl::net::redis::Subtree topicRoot();

        /** Access last topic Id.
            Contains newest message Id and is incremented for each new topic.
            \return key */
        afl::net::redis::IntegerKey lastTopicId();

        /** Access root of "forum" tree.
            \return tree
            \see Forum */
        afl::net::redis::Subtree forumRoot();

        /** Access last forum Id.
            Contains newest forum Id and is incremented for each new forum.
            \return key */
        afl::net::redis::IntegerKey lastForumId();

        /** Access set of all forums.
            \return key */
        afl::net::redis::IntegerSetKey allForums();

        /** Access newsgroup-to-forum map.
            Maps newsgroup names (string) to forum Ids (integers).
            \return key */
        afl::net::redis::HashKey newsgroupMap();

        /** Access well-known-forum map.
            Maps well-known forum names (string) to forum Ids (integers).
            \return key
            \see TalkForum::findForum() */
        afl::net::redis::HashKey forumMap();

        /** Access root of "email" tree.
            This tree is used for generating email addressess on NNTP.
            \return tree */
        afl::net::redis::Subtree emailRoot();

        /** Access default folder definitions.
            Default folder definitions can be overridden by user definitions (User::pmFolderData()).
            \return tree */
        afl::net::redis::Subtree defaultFolderRoot();

        /** Access root of PM tree.
            \return tree
            \see UserPM */
        afl::net::redis::Subtree pmRoot();

        /** Access root of RfC message-Id tree.
            Contains a StringKey for each RfC message-Id.
            \return tree */
        afl::net::redis::Subtree rfcMessageIdRoot();

        /** Check a user's permissions.
            Most code will use Session::hasPermission/Session::checkPermission, but notify needs to verify permissions without a session.
            Privilege strings are a comma-separated list of items.
            If a user matches an item, they have the permission; if that item is preceded by a "-", they don't.
            - all: always match
            - p:XX: match users that have integer > 0 in their profile as XX
            - u:XX: match user Id XX
            - g:XX: match users that are on game XX
            \param privString Privilege string
            \param user User to check privileges for
            \return true if user has requested permissions */
        bool checkUserPermission(String_t privString, String_t user);

        /** Check whether user is on an active game.
            \param userId User Id
            \param gameNumber Game number */
        bool isUserOnActiveGame(String_t userId, int32_t gameNumber);

     private:
        afl::sys::Mutex m_mutex;
        afl::sys::Log m_log;

        util::syntax::KeywordTable m_keywordTable;
        InlineRecognizer m_recognizer;
        LinkFormatter m_linkFormatter;

        afl::net::CommandHandler& m_db;

        Configuration m_config;
        std::auto_ptr<Notifier> m_pNotifier;
    };

} }

#endif
