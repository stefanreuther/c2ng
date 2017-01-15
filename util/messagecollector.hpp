/**
  *  \file util/messagecollector.hpp
  */
#ifndef C2NG_UTIL_MESSAGECOLLECTOR_HPP
#define C2NG_UTIL_MESSAGECOLLECTOR_HPP

#include "afl/sys/loglistener.hpp"
#include "afl/sys/mutex.hpp"
#include "util/messagematcher.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/base/types.hpp"

namespace util {

    /** Log message collector.
        This is a LogListener that collects and stores messages for later retrieval.
        It provides a means of filtering that can either discard messages, or store but hide them;
        reconfiguration may make hidden messages visible, but not discarded ones. */
    class MessageCollector : public afl::sys::LogListener {
     public:
        /** Message sequence number.
            Conceptually, points between two messages; readNewerMessage() reads the next one after, readOlderMessage() reads the next one before. */
        typedef uint64_t MessageNumber_t;

        /** Constructor. */
        MessageCollector();

        /** Destructor. */
        virtual ~MessageCollector();

        // LogListener
        virtual void handleMessage(const Message& msg);

        /** Configure the MessageCollector.
            \param filter Filter string for MessageMatcher::setConfiguration(). Should produce "keep", "hide", or "drop" commands. */
        void setConfiguration(String_t filter);

        /** Get oldest position in message buffer. */
        MessageNumber_t getOldestPosition() const;

        /** Get newest position in message buffer. */
        MessageNumber_t getNewestPosition() const;

        /** Read newer message.
            Reads the first message at position >= startAt and advances pointer after that.
            \param startAt Initial position
            \param storeMessage If non-null, message is stored here
            \param next [out] New position
            \retval true Newer message found and result has been produced
            \retval false No newer message exists */
        bool readNewerMessage(MessageNumber_t startAt, Message* storeMessage, MessageNumber_t& next) const;

        /** Read older message.
            Reads the first message at position < startAt and advances pointer after that.
            \param startAt Initial position
            \param storeMessage If non-null, message is stored here
            \param next [out] New position
            \retval true Older message found and result has been produced
            \retval false No older message exists  */
        bool readOlderMessage(MessageNumber_t startAt, Message* storeMessage, MessageNumber_t& next) const;

     private:
        // Mutex for everything
        afl::sys::Mutex m_mutex;

        // Configuration
        MessageMatcher m_config;

        // Content
        struct Item {
            Message message;
            bool visible;
            Item(const Message& msg, bool visible)
                : message(msg), visible(visible)
                { }
        };
        afl::container::PtrVector<Item> m_messages;
        MessageNumber_t m_firstMessageNumber;
    };

}

#endif
