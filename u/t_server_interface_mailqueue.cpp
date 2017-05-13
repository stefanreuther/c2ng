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
    };
    Tester t;
}

