/**
  *  \file server/mailout/mailqueue.hpp
  *  \brief Class server::mailout::MailQueue
  */
#ifndef C2NG_SERVER_MAILOUT_MAILQUEUE_HPP
#define C2NG_SERVER_MAILOUT_MAILQUEUE_HPP

#include "server/interface/mailqueue.hpp"

namespace server { namespace mailout {

    class Session;
    class Root;
    class Message;

    /** Implementation of the MailQueue interface for c2mailout.
        This is the (possibly short-lived) command interpreter.
        All state lives in the Root and Session. */
    class MailQueue : public server::interface::MailQueue {
     public:
        /** Constructor.
            \param root Service root
            \param session Session state */
        MailQueue(Root& root, Session& session);

        // MailQueue:
        virtual void startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId);
        virtual void addParameter(String_t parameterName, String_t value);
        virtual void addAttachment(String_t url);
        virtual void send(afl::base::Memory<const String_t> receivers);
        virtual void cancelMessage(String_t uniqueId);
        virtual void confirmAddress(String_t address, String_t key, afl::base::Optional<String_t> info);
        virtual void requestAddress(String_t user);
        virtual void runQueue();

     private:
        Root& m_root;
        Session& m_session;

        Message& currentMessage();
    };

} } 

#endif
