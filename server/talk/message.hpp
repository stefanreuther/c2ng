/**
  *  \file server/talk/message.hpp
  *  \brief Class server::talk::Message
  */
#ifndef C2NG_SERVER_TALK_MESSAGE_HPP
#define C2NG_SERVER_TALK_MESSAGE_HPP

#include "afl/net/redis/subtree.hpp"
#include "afl/base/types.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "server/interface/talkpost.hpp"
#include "server/talk/sorter.hpp"
#include "afl/data/hash.hpp"

namespace server { namespace talk {

    class Root;
    class Topic;

    // FIXME: rename to Post for consistency with network interface?

    /** Message.
        Represents a message in a forum thread.
        A message contains a header with metainformation and the message body.
        The header includes authorship and subject information as well as information to generate a NNTP view.

        Supporting NNTP means we have to be able to generate RfC Message Ids or preserve the Ids given by users, and to track a sequence number.
        Also, when a posting is edited, we have to update the Ids and retain the previous ones
        to make the edit visible to NNTP clients and to generate a Supersedes header.

        A message is identified by a message Id, a nonzero integer.
        (In this program, "message Id" always refers to that integer; the NNTP Message Id is referenced as "RfC Message Id".) */
    class Message {
     public:
        /** Constructor.
            \param root Service root
            \param messageId Message Id */
        Message(Root& root, int32_t messageId);

        /** Access message header.
            This accesses the raw header (not recommended normally).
            \return header */
        afl::net::redis::HashKey header();

        /** Access topic Id.
            \return Topic Id of the topic this message is in. */
        afl::net::redis::IntegerField topicId();

        /** Access parent message.
            \return Message Id of parent message (=the message this one is a reply to).
            Zero if this is not a reply. */
        afl::net::redis::IntegerField parentMessageId();

        /** Access post time.
            \return Initial submission time of this posting (=Time_t). */
        afl::net::redis::IntegerField postTime();

        /** Access edit time.
            \return Time of last edit (=Time_t), if any. 0 if post was never edited. */
        afl::net::redis::IntegerField editTime();

        /** Access author of message.
            \return User Id */
        afl::net::redis::StringField author();

        /** Access subject of message.
            \return Subject */
        afl::net::redis::StringField subject();

        /** Access RfC Message Id of message, if present.
            This field contains the user-supplied RfC Message Id.
            Normally, you use getRfcMessageId() to obtain the effective RfC Message Id.
            \return RfC Message Id if present, empty string if none set */
        afl::net::redis::StringField rfcMessageId();

        /** Access RfC headers.
            This field contains the entire RfC headers of the message, if present.

            FIXME: this is not yet implemented and therefore the detail format not yet specified.

            \return Headers */
        afl::net::redis::StringField rfcHeaders();

        /** Access sequence number.
            The sequence number increases whenever a message is created or modified in a forum,
            see Forum::lastMessageSequenceNumber().
            \return Sequence number */
        afl::net::redis::IntegerField sequenceNumber();

        /** Access previous sequence number.
            If this post is edited, the previous sequence number is stored here.
            \return Previous sequence number */
        afl::net::redis::IntegerField previousSequenceNumber();

        /** Access previous RfC Message Id, if present.
            If this post is edited, the previous RfC Message Id is stored here.
            Normally, you use getPreviousRfcMessageId() to obtain the previous effective RfC Message Id.
            \return Previous RfC Message Id if present, empty string if none set */
        afl::net::redis::StringField previousRfcMessageId();

        /** Access message text.
            This is the message text in a format suitable for rendering
            (that is, including the type prefix, e.g. "text:").
            \return text */
        afl::net::redis::StringKey text();

        /** Check existance.
            \return true if this message exists. */
        bool exists();

        /** Remove this message.
            Removes the message from its topic and forum, and from the database.
            \param root Service root */
        void remove(Root& root);

        /** Access message topic.
            This is a convenience function for creating a Topic object
            \param root Service root
            \return Topic object corresponding to topicId() */
        Topic topic(Root& root);

        /** Get message Id.
            \return Message Id */
        int32_t getId() const;

        /** Describe message.
            \param root Service root
            \return information */
        server::interface::TalkPost::Info describe(const Root& root);

        /** Get RfC Message Id.
            If the message has a user-supplied message Id, returns that.
            Otherwise, returns a synthetic message Id created from configuration and the message's metadata.
            \param root Service root
            \return RfC Message Id */
        String_t getRfcMessageId(const Root& root);

        /** Get Previous RfC Message Id.
            Like getRfcMessageId(), but returns the RfC Message Id before the last edit.
            \param root Service root
            \return Previous RfC Message Id, empty if there was no previous edit */
        String_t getPreviousRfcMessageId(const Root& root);

        /** Get RfC header.
            Creates the message headers in a hash.
            In addition to the actual headers, this will contain pseudo-headers:
            - :Seq (sequence number)
            - :Bytes (estimated message size in bytes)
            - :Lines (estimated message size in lines)
            - :Id (message Id [the integer])
            \param root Service root
            \return Hash containing header information */
        afl::data::Hash::Ref_t getRfcHeader(Root& root);


        /** Remove RfC Message Id.
            Called when a message with RfC Message Id is removed or edited,
            to make sure the message Id is no longer resolvable.
            \param root Service root
            \param id RfC Message Id */
        static void removeRfcMessageId(Root& root, String_t id);

        /** Add RfC Message Id.
            Called when a message with RfC Message Id is created,
            to make sure the message Id is resolvable.
            \param root Service root
            \param id RfC Message Id
            \param messageId Message Id [the integer] */
        static void addRfcMessageId(Root& root, String_t id, int32_t messageId);

        /** Look up a RfC Message Id.
            This function can resolve message Ids as given by getRfcMessageId(), i.e. synthetic and user-supplied ones.
            \param root Service root
            \param rfcMsgId RfC Message Id
            \return Message Id [the integer] if found, 0 if not found */
        static int32_t lookupRfcMessageId(Root& root, String_t rfcMsgId);


        /** Apply sort-by-sequence.
            Given a SortOperation on a set-of-Message-Ids, requests that set to be sorted by sequence numbers.
            \param root [in] Service root
            \param op [in/out] Sort Operation */
        static void applySortBySequence(Root& root, afl::net::redis::SortOperation& op);

        /** Apply sort-by-sequence and return sequence numbers.
            Given a SortOperation on a set-of-Message-Ids, requests that set to be sorted by sequence numbers,
            and to return sequence numbers and Message Ids.
            \param root [in] Service root
            \param op [in/out] Sort Operation */
        static void applySortBySequenceMap(Root& root, afl::net::redis::SortOperation& op);

        /** Message sorter.
            Pass this object to executeListOperation() if the list contains a list of forum messages. */
        class MessageSorter : public Sorter {
         public:
            MessageSorter(Root& root);
            virtual void applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const;
         private:
            Root& m_root;
        };

     private:
        afl::net::redis::Subtree m_message;
        int32_t m_messageId;
    };

} }

#endif
