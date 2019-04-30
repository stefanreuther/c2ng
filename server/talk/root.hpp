/**
  *  \file server/talk/root.hpp
  *  \brief Class server::talk::Root
  */
#ifndef C2NG_SERVER_TALK_ROOT_HPP
#define C2NG_SERVER_TALK_ROOT_HPP

#include "afl/net/commandhandler.hpp"
#include "afl/net/redis/integerkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/sys/log.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/inlinerecognizer.hpp"
#include "server/talk/linkformatter.hpp"
#include "server/types.hpp"
#include "util/syntax/keywordtable.hpp"
#include "server/common/root.hpp"

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
            \param mail Mail connection
            \param config Configuration. Will be copied. */
        Root(afl::net::CommandHandler& db, afl::net::CommandHandler& mail, const Configuration& config);

        /** Destructor. */
        ~Root();

        /*
         *  Nested Objects
         */

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

        /** Access mail queue service.
            \return mail queue service */
        server::interface::MailQueue& mailQueue();

        /** Get current time.
            The time is specified in minutes-since-epoch.
            \return time */
        Time_t getTime();

        /*
         *  Database Layout
         */

        afl::net::redis::Subtree groupRoot();
        afl::net::redis::Subtree messageRoot();
        afl::net::redis::IntegerKey lastMessageId();

        afl::net::redis::Subtree topicRoot();
        afl::net::redis::IntegerKey lastTopicId();

        afl::net::redis::Subtree forumRoot();
        afl::net::redis::IntegerKey lastForumId();
        afl::net::redis::IntegerSetKey allForums();
        afl::net::redis::HashKey newsgroupMap();
        afl::net::redis::HashKey forumMap();

        afl::net::redis::Subtree emailRoot();

        afl::net::redis::Subtree defaultFolderRoot();
        afl::net::redis::Subtree pmRoot();
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

     private:
        afl::sys::Log m_log;

        util::syntax::KeywordTable m_keywordTable;
        InlineRecognizer m_recognizer;
        LinkFormatter m_linkFormatter;

        afl::net::CommandHandler& m_db;
        server::interface::MailQueueClient m_mailQueue;

        Configuration m_config;
    };

} }

#endif
