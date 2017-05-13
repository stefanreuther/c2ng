/**
  *  \file server/talk/forum.hpp
  *  \brief Class server::talk::Forum
  */
#ifndef C2NG_SERVER_TALK_FORUM_HPP
#define C2NG_SERVER_TALK_FORUM_HPP

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "server/interface/talkforum.hpp"
#include "server/interface/talknntp.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"
#include "server/talk/sorter.hpp"

namespace server { namespace talk {

    class Root;
    class Session;

    /** Forum.
        Represents access to a single forum.
        A forum contains
        - metainformation to produce a web and a NNTP view
        - links to normal and sticky topics
        - links to watchers
        - permissions

        A forum is identified by a forum Id, a nonzero integer. */
    class Forum {
     public:
        /** Constructor.
            \param root Service root
            \param forumId Forum Id */
        Forum(Root& root, int32_t forumId);

        /*
         *  Database access
         */

        /** Access header.
            \return header */
        afl::net::redis::HashKey header();

        /** Access forum name.
            Format: string.
            \return forum name field */
        afl::net::redis::StringField name();

        /** Access forum description.
            Format: a string suitable for rendering (i.e. type prefix + text).
            \return forum description field. */
        afl::net::redis::StringField description();

        /** Access read permissions.
            Read permission allows users to view the forum content unless topic permissions forbid that.
            Format: a comma-separated list of permissions. See Root::checkUserPermission.
            \return read permission field */
        afl::net::redis::StringField readPermissions();

        /** Access write permissions.
            Write permission allows users to create new topics.
            Format: a comma-separated list of permissions. See Root::checkUserPermission.
            \return write permission field */
        afl::net::redis::StringField writePermissions();

        /** Access answer permissions.
            Answer permission allows users to write replies in existing topics unless topic permissions forbid that.
            Format: a comma-separated list of permissions. See Root::checkUserPermission.
            \return answer permission field */
        afl::net::redis::StringField answerPermissions();

        /** Access delete permissions.
            Write permission allows users to delete messages.
            Format: a comma-separated list of permissions. See Root::checkUserPermission.
            \return delete permission field */
        afl::net::redis::StringField deletePermissions();

        /** Access sort key.
            This is used to sort forums.
            \return sort key field */
        afl::net::redis::StringField key();

        /** Access last message sequence number.
            This number increases for every new or modified post.
            It is required for the NNTP view.
            \return last message sequence number field */
        afl::net::redis::IntegerField lastMessageSequenceNumber();

        /** Access creation time.
            Format: Time_t
            \return creation time field. */
        afl::net::redis::IntegerField creationTime();

        /** Access last post Id.
            Format: a post Id
            \return last post Id field */
        afl::net::redis::IntegerField lastPostId();

        /** Access last forum modification time.
            This time is updated on every change to the forum content.
            Format: Time_t.
            \return last forum modification time field */
        afl::net::redis::IntegerField lastTime();

        /** Access messages.
            This set contains the post Ids of all messages.
            \return message field */
        afl::net::redis::IntegerSetKey messages();

        /** Access topics.
            This set contains all topic Ids of all topics in this forum.
            \return topics field */
        afl::net::redis::IntegerSetKey topics();

        /** Access sticky topics.
            This set contains all sticky topic Ids of all topics in this forum.
            Sticky topics are intended to be rendered on top of the forum.
            \return sticky topics field */
        afl::net::redis::IntegerSetKey stickyTopics();

        /** Access watchers.
            This set contains the user Ids of all users watching this forum.
            \return watchers field */
        afl::net::redis::StringSetKey watchers();

        /*
         *  Other Operations
         */

        /** Set parent group.
            Changing the parent group must update the group's link to this forum.
            \param newParent new parent group name; "" to make this forum not be in a group
            \param root Service root */
        void setParent(String_t newParent, Root& root);

        /** Get parent group.
            \return parent group name (GRID). */
        String_t getParent();

        /** Set newsgroup name.
            Changes this forum's name on the NNTP side.
            This must update the forum-to-newsgroup mapping and resolve name conflicts.
            \param newNG Newsgroup name
            \param root Service root */
        void setNewsgroup(String_t newNG, Root& root);

        /** Get newsgroup name.
            \return Newsgroup name; "" if none */
        String_t getNewsgroup();

        /** Get forum Id.
            \return Id */
        int32_t getId() const;

        /** Check existence.
            \param root Service root
            \return true if forum exists */
        bool exists(Root& root);

        /** Describe for forum side.
            \param ctx Render context to render the description
            \param opts Render options to render the description
            \param root Service root
            \return information */
        server::interface::TalkForum::Info describe(const server::talk::render::Context& ctx, const server::talk::render::Options& opts, Root& root);

        /** Describe for NNTP side.
            \param ctx Render context to render the description
            \param opts Render options to render the description
            \param root Service root
            \param session Session to resolve permissions
            \return information */
        server::interface::TalkNNTP::Info describeAsNewsgroup(const server::talk::render::Context& ctx, const server::talk::render::Options& opts, Root& root, Session& session);

        /** Forum sorter.
            Pass this object to executeListOperation() if the list contains a list of forums. */
        class ForumSorter : public Sorter {
         public:
            ForumSorter(Root& root);
            virtual void applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const;
         private:
            Root& m_root;
        };

     private:
        afl::net::redis::Subtree m_forum;
        int32_t m_forumId;
    };

} }

#endif
