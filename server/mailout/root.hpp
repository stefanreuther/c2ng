/**
  *  \file server/mailout/root.hpp
  *  \brief Class server::mailout::Root
  */
#ifndef C2NG_SERVER_MAILOUT_ROOT_HPP
#define C2NG_SERVER_MAILOUT_ROOT_HPP

#include <memory>
#include "afl/net/redis/subtree.hpp"
#include "afl/sys/log.hpp"
#include "server/common/root.hpp"
#include "server/mailout/configuration.hpp"
#include "server/interface/mailqueue.hpp"

namespace server { namespace mailout {

    class Message;
    class Transmitter;

    /** A mailout server's root state.
        Contains global configuration and state objects.
        Root is shared between all connections.

        Root contains the top-level database layout rules.
        All accesses happen through subtree or other objects given out by Root.

        Root also contains higher-level methods to allocate messages and verify email addresses. */
    class Root : public server::common::Root {
     public:
        /** Constructor.
            \param db Database
            \param config Configuration (will be copied) */
        explicit Root(afl::net::CommandHandler& db, const Configuration& config);

        /** Access logger.
            \return logger */
        afl::sys::Log& log();

        /** Set transmitter.
            A possible transmitter must be valid for the lifetime of the Root,
            or be unregistered before dying.
            \param p Transmitter instance (can be null). */
        void setTransmitter(Transmitter* p);

        /** Get transmitter.
            \return Transmitter instance (can be null). */
        Transmitter* getTransmitter();

        /** Access database root for message storage.
            \return database node */
        afl::net::redis::Subtree mailRoot();

        /** Access Message Unique Id mapping.
            \return hash of unique Id to message */
        afl::net::redis::HashKey uniqueIdMap();

        /** Access set of messages being prepared (partial messages).
            \return database node */
        afl::net::redis::IntegerSetKey preparingMessages();

        /** Access set of messages being sent (complete messages).
            \return database node */
        afl::net::redis::IntegerSetKey sendingMessages();

        /** Access configuration.
            \return configuration. */
        const Configuration& config() const;


        /*
         *  Higher-Level Methods
         */

        /** Prepare mail queues.
            Clears the "pending" queue and call send() on all elements of the "sending" queue.
            Call this once after startup. */
        void prepareQueues();

        /** Allocate a message.
            Use this when sending new mail.
            The message is created in state "pending".
            After allocating the message, you must
            - set its properties
            - send it on database level (Message::send)
            - send it on transmitter level (Transmitter::send)
            \return newly-allocated message, never null */
        std::auto_ptr<server::mailout::Message> allocateMessage();

        /** Resolve an address.
            \param address     [in] Address from message queue (see Message::receivers())
            \param smtpAddress [out] Resolved SMTP address if any
            \param authUser    [out] Authenticated user Id for further access checks
            \retval true Address resolved correctly, send the message
            \retval false Address temporarily failed, postpone the message
            \throw std::runtime_error Address permanently failed, discard the message */
        bool resolveAddress(String_t address, String_t& smtpAddress, String_t& authUser);

        /** Confirm an email address.
            \param mail    Mail address
            \param key     Received confirmation key
            \param info    Tracking information to store (free-form text)
            \return true if confirmation was accepted, false if key was invalid */
        bool confirmMail(String_t mail, String_t key, String_t info);

        /** Clean up expired Ids. */
        void cleanupUniqueIdMap();

        /** Get user's email status.
            \param user User Id
            \return status */
        server::interface::MailQueue::UserStatus getUserStatus(String_t user);

        /** Get current time.
            We store MINUTES since epoch.
            \return time */
        int32_t getCurrentTime();

     private:
        afl::net::CommandHandler& m_db;
        Configuration m_config;
        afl::sys::Log m_log;
        Transmitter* m_transmitter;

        void requestConfirmation(String_t user, String_t userEmail);
    };

} }

inline afl::sys::Log&
server::mailout::Root::log()
{
    return m_log;
}

inline void
server::mailout::Root::setTransmitter(Transmitter* p)
{
    m_transmitter = p;
}

inline server::mailout::Transmitter*
server::mailout::Root::getTransmitter()
{
    return m_transmitter;
}

inline const server::mailout::Configuration&
server::mailout::Root::config() const
{
    return m_config;
}

#endif
