/**
  *  \file server/mailout/message.hpp
  *  \brief Class server::mailout::Message
  */
#ifndef C2NG_SERVER_MAILOUT_MESSAGE_HPP
#define C2NG_SERVER_MAILOUT_MESSAGE_HPP

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/string.hpp"

namespace server { namespace mailout {

    class Root;

    /** A message.
        Represents a message that is being worked on, to access its database properties.

        <b>Lifecycle of a Message</b>

        (a) after creation:
        - in preparingMessages()
        - not in uniqueIdMap()
        - data partially populated

        (b.1) when sending (Message::send()):
        - in preparingMessages()
        - in uniqueIdMap()
        - data completely populated

        (b.2)
        - in sendingMessages()
        - in uniqueIdMap()
        - data completely populated
        Note that send() is not atomic making this two states.

        (c) after MailQueue::cancelMessage():
        - in sendingMessages()
        - not in uniqueIdMap()
        - data completely populated
        If transmitter handles this message, it will discard it.

        (d) after transmission (Message::remove()):
        - not in sendingMessages()
        - in uniqueIdMap()
        - data removed */
    class Message {
     public:
        enum State {
            Preparing,
            Sending
        };

        /** Constructor.
            \param root  Service root
            \param mid   Message Id
            \param state State of this message */
        Message(Root& root, int32_t mid, State state);

        /** Access template name.
            \return database handle */
        afl::net::redis::StringField templateName();

        /** Access message unique identifier.
            \return database handle */
        afl::net::redis::StringField uniqueId();

        /** Access message parameter hash.
            \return database handle */
        afl::net::redis::HashKey arguments();

        /** Access message attachment list.
            \return database handle */
        afl::net::redis::StringListKey attachments();

        /** Access message receiver set.
            Each receiver is a pair of addressing scheme and address,
            as in "user:<uid>" or "mail:<user>@<host>".
            \return database handle */
        afl::net::redis::StringSetKey receivers();

        /** Access message expiration time.
            \return database handle */
        afl::net::redis::IntegerField expireTime();

        /** Remove message from database. */
        void remove();

        /** Prepare message for sending.
            Moves the message into the sending queue, and makes sure that no other message with the same uniqueId() will be sent.

            Note that this only updates the database copy of the send queue;
            it does NOT place the message in the in-memory send queue of the transmitter;
            you must call Transmitter::send(getId()) after calling this function. */
        void send();

        /** Get message Id.
            \return message Id */
        int32_t getId() const;

     private:
        Root& m_root;
        afl::net::redis::Subtree m_message;
        int32_t m_messageId;
        State m_state;
    };

} }

// Get message Id.
inline int32_t
server::mailout::Message::getId() const
{
    return m_messageId;
}

#endif
