/**
  *  \file server/mailout/transmitter.hpp
  *  \brief Interface server::mailout::Transmitter
  */
#ifndef C2NG_SERVER_MAILOUT_TRANSMITTER_HPP
#define C2NG_SERVER_MAILOUT_TRANSMITTER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"

namespace server { namespace mailout {

    /** Interface for a transmitter.
        The transmitter's job is to (try to) empty the "sending" queue.
        Mailout can run without a transmitter, which means it only processes commands
        and places them in the send queue, but never sends them. */
    class Transmitter : public afl::base::Deletable {
     public:
        /** Send a message.
            Called when a new message has been placed in the send queue.
            \param messageId Message Id */
        virtual void send(int32_t messageId) = 0;

        /** Notify change of the status of an address.
            This can cause messages from the sending queue to become ready for sending.
            \param address Email address */
        virtual void notifyAddress(String_t address) = 0;

        /** Reconsider queue.
            Called after an unspecified change in environment that can cause messages to become ready for sending. */
        virtual void runQueue() = 0;
    };

} }

#endif
