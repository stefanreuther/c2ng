/**
  *  \file server/test/mailmock.hpp
  *  \brief Class server::test::MailMock
  */
#ifndef C2NG_SERVER_TEST_MAILMOCK_HPP
#define C2NG_SERVER_TEST_MAILMOCK_HPP

#include <memory>
#include <map>
#include <set>
#include "afl/container/ptrvector.hpp"
#include "afl/test/assert.hpp"
#include "server/interface/mailqueue.hpp"

namespace server { namespace test {

    /** Mail Mock.

        This simulates a mail queue.
        It verifies the command sequence.
        It stashes away received messages,
        with their parameters and attachments.

        To use, use code-under test, then repeatedly
        - extract and inspect messages to a specific receiver using extract()
        - extract and inspect messages in sequential order using extractFirst() */
    class MailMock : public server::interface::MailQueue {
     public:
        struct Message {
            String_t templateName;
            std::map<String_t, String_t> parameters;
            std::set<String_t> attachments;
            std::set<String_t> receivers;

            bool hasAttachment(String_t what) const
                { return attachments.find(what) != attachments.end(); }
        };

        /** Constructor.
            @param a Asserter */
        MailMock(afl::test::Assert a);

        // MailQueue methods:
        virtual void startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId);
        virtual void addParameter(String_t parameterName, String_t value);
        virtual void addAttachment(String_t url);
        virtual void send(afl::base::Memory<const String_t> receivers);
        virtual void cancelMessage(String_t uniqueId);
        virtual void confirmAddress(String_t address, String_t key, afl::base::Optional<String_t> info);
        virtual void requestAddress(String_t user);
        virtual void runQueue();
        virtual UserStatus getUserStatus(String_t user);

        /** Extract message by receiver.
            Looks for a message to that receiver, strikes it out of that message's receiver field
            (such that the next call will not return it again),
            and returns the message.

            Note that contrary to its name, this method does not transfer ownership of the message.

            @param receiver Receiver

            @return Message to that receiver. Null if none. */
        Message* extract(String_t receiver);

        /** Extract first message.
            Call repeatedly to access all messages.

            Note that unlike extract(), this method DOES transfer ownership.

            @return Message. Caller assumes ownership. Null if none. */
        std::auto_ptr<Message> extractFirst();

        /** Check emptiness of queue.
            @return true if queue is empty (no messages sent / all messages consumed). */
        bool empty() const;

     private:
        afl::test::Assert m_assert;
        std::auto_ptr<Message> m_current;
        afl::container::PtrVector<Message> m_queue;
    };

} }

#endif
