/**
  *  \file server/talk/topic.hpp
  *  \brief Class server::talk::Topic
  */
#ifndef C2NG_SERVER_TALK_TOPIC_HPP
#define C2NG_SERVER_TALK_TOPIC_HPP

#include "afl/base/types.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "server/interface/talkthread.hpp"
#include "server/talk/sorter.hpp"

namespace server { namespace talk {

    class Root;
    class Forum;

    // FIXME: rename to 'Thread' now that we've namespaces?

    /** A discussion thread.
        Called "Topic" because operating systems also have threads.

        A topic contains a header with metainformation and links to the messages.
        A topic allows access control.

        A topic is identified by a topic Id, a nonzero integer.

        Topics can be sticky or normal.
        From the service's point of view, this is just a division between two types of topics, with no difference in behaviour.
        Sticky topics are intended to be displayed differently (always-on-top) by the front-end. */
    class Topic {
     public:
        /** Constructor.
            \param root Service root
            \param topicId Topic Id */
        Topic(Root& root, int32_t topicId);

        /** Access topic header.
            This accesses the raw header (not recommended normally).
            \return header */
        afl::net::redis::HashKey header();

        /** Access topic subject.
            \return topic subject */
        afl::net::redis::StringField subject();

        /** Access topic's forum Id.
            \return forum Id */
        afl::net::redis::IntegerField forumId();

        /** Access topic's first posting Id.
            Note that this message may not exist anymore if it has been deleted.
            \return first posting Id (a Message Id) */
        afl::net::redis::IntegerField firstPostingId();

        /** Access topic's read permissions.
            This field need not be set if the forum's read permissions are to be used instead.
            \return read permissions */
        afl::net::redis::StringField readPermissions();

        /** Access topic's answer permissions.
            This field need not be set if the forum's answer or write permissions are to be used instead.
            \return answer permissions */
        afl::net::redis::StringField answerPermissions();

        /** Access topic's last posting Id.
            Note that this message may not exist anymore if it has been deleted.
            \return last posting Id (a Message Id) */
        afl::net::redis::IntegerField lastPostId();

        /** Access topic's last modification time.
            \return last modification time (a Time_t) */
        afl::net::redis::IntegerField lastTime();

        /** Access topic's messages.
            This set contains all Ids of messages of this topics.
            \return messages */
        afl::net::redis::IntegerSetKey messages();

        /** Access topic's watchers.
            This set contains the user Ids of all users watching this topic.
            \return watchers */
        afl::net::redis::StringSetKey watchers();

        /** Access topic's forum.
            This is a convenience function for creating a Topic object.
            \param root Service root
            \return Forum object corresponding to forumId() */
        Forum forum(Root& root);

        /** Remove this topic.
            Removes all messages.
            \param root Service root */
        void remove(Root& root);

        /** Remove this empty topic.
            Must only be called for empty topics.
            Effectively, the only legitimate caller for this function is Message::remove
            (possibly indirectly via Topic::remove).
            \param root Service root */
        void removeEmpty(Root& root);

        /** Check existence.
            \return true if this topic exists */
        bool exists();

        /** Describe topic.
            \return information */
        server::interface::TalkThread::Info describe();

        /** Check stickyness.
            \return true if this topic is marked sticky */
        bool isSticky();

        /** Set stickyness.
            \param root Service root
            \param enable true to make sticky, false to make normal */
        void setSticky(Root& root, bool enable);

        /** Get topic Id.
            \return topic Id */
        int32_t getId() const;

        /** Topic sorter.
            Pass this object to executeListOperation() if the list contains a list of topic Ids. */
        class TopicSorter : public Sorter {
         public:
            TopicSorter(Root& root);
            virtual void applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const;
         private:
            Root& m_root;
        };

     private:
        afl::net::redis::Subtree m_topic;
        int32_t m_topicId;
    };

} }

#endif
