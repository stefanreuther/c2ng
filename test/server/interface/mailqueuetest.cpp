/**
  *  \file test/server/interface/mailqueuetest.cpp
  *  \brief Test for server::interface::MailQueue
  */

#include "server/interface/mailqueue.hpp"
#include "afl/test/testrunner.hpp"

using server::interface::MailQueue;

/** Interface test. */
AFL_TEST_NOARG("server.interface.MailQueue:interface")
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
AFL_TEST("server.interface.MailQueue:parseAddressStatus", a)
{
    a.checkEqual("01", MailQueue::parseAddressStatus(""),  MailQueue::NotSet);
    a.checkEqual("02", MailQueue::parseAddressStatus("u"), MailQueue::Unconfirmed);
    a.checkEqual("03", MailQueue::parseAddressStatus("c"), MailQueue::Confirmed);
    a.checkEqual("04", MailQueue::parseAddressStatus("b"), MailQueue::Blocked);
    a.checkEqual("05", MailQueue::parseAddressStatus("r"), MailQueue::Requested);
}

AFL_TEST("server.interface.MailQueue:formatAddressStatus", a)
{
    a.checkEqual("01", "",  MailQueue::formatAddressStatus(MailQueue::NotSet));
    a.checkEqual("02", "u", MailQueue::formatAddressStatus(MailQueue::Unconfirmed));
    a.checkEqual("03", "c", MailQueue::formatAddressStatus(MailQueue::Confirmed));
    a.checkEqual("04", "b", MailQueue::formatAddressStatus(MailQueue::Blocked));
    a.checkEqual("05", "r", MailQueue::formatAddressStatus(MailQueue::Requested));
}
