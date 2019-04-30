/**
  *  \file u/t_server_interface_mailqueue.cpp
  *  \brief Test for server::interface::MailQueue
  */

#include "server/interface/mailqueue.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceMailQueue::testInterface()
{
    class Tester : public server::interface::MailQueue {
     public:
        virtual void startMessage(String_t /*templateName*/, afl::base::Optional<String_t> /*uniqueId*/)
            { }
        virtual void addParameter(String_t /*parameterName*/, String_t /*value*/)
            { }
        virtual void addAttachment(String_t /*url*/)
            { }
        virtual void send(afl::base::Memory<const String_t> /*receivers*/)
            { }
        virtual void cancelMessage(String_t /*uniqueId*/)
            { }
        virtual void confirmAddress(String_t /*address*/, String_t /*key*/, afl::base::Optional<String_t> /*info*/)
            { }
        virtual void requestAddress(String_t /*user*/)
            { }
        virtual void runQueue()
            { }
        virtual UserStatus getUserStatus(String_t /*user*/)
            { return UserStatus(); }
    };
    Tester t;
}

/** Test parseAddressStatus(), formatAddressStatus(). */
void
TestServerInterfaceMailQueue::testAddressStatus()
{
    using server::interface::MailQueue;

    TS_ASSERT_EQUALS(MailQueue::parseAddressStatus(""),  MailQueue::NotSet);
    TS_ASSERT_EQUALS(MailQueue::parseAddressStatus("u"), MailQueue::Unconfirmed);
    TS_ASSERT_EQUALS(MailQueue::parseAddressStatus("c"), MailQueue::Confirmed);
    TS_ASSERT_EQUALS(MailQueue::parseAddressStatus("b"), MailQueue::Blocked);
    TS_ASSERT_EQUALS(MailQueue::parseAddressStatus("r"), MailQueue::Requested);

    TS_ASSERT_EQUALS("",  MailQueue::formatAddressStatus(MailQueue::NotSet));
    TS_ASSERT_EQUALS("u", MailQueue::formatAddressStatus(MailQueue::Unconfirmed));
    TS_ASSERT_EQUALS("c", MailQueue::formatAddressStatus(MailQueue::Confirmed));
    TS_ASSERT_EQUALS("b", MailQueue::formatAddressStatus(MailQueue::Blocked));
    TS_ASSERT_EQUALS("r", MailQueue::formatAddressStatus(MailQueue::Requested));

}

