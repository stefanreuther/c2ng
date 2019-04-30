/**
  *  \file server/interface/mailqueueclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_MAILQUEUECLIENT_HPP
#define C2NG_SERVER_INTERFACE_MAILQUEUECLIENT_HPP

#include "server/interface/mailqueue.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class MailQueueClient : public MailQueue {
     public:
        MailQueueClient(afl::net::CommandHandler& commandHandler);
        ~MailQueueClient();

        virtual void startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId);
        virtual void addParameter(String_t parameterName, String_t value);
        virtual void addAttachment(String_t url);
        virtual void send(afl::base::Memory<const String_t> receivers);
        virtual void cancelMessage(String_t uniqueId);
        virtual void confirmAddress(String_t address, String_t key, afl::base::Optional<String_t> info);
        virtual void requestAddress(String_t user);
        virtual void runQueue();
        virtual UserStatus getUserStatus(String_t user);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
