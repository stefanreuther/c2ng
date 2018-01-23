/**
  *  \file server/mailin/mailprocessor.hpp
  *  \brief Class server::mailin::MailProcessor
  */
#ifndef C2NG_SERVER_MAILIN_MAILPROCESSOR_HPP
#define C2NG_SERVER_MAILIN_MAILPROCESSOR_HPP

#include "afl/net/commandhandler.hpp"
#include "afl/net/mimeparser.hpp"
#include "afl/sys/loglistener.hpp"
#include "server/interface/mailqueue.hpp"

namespace server { namespace mailin {

    /** Mail processor.
        Encapsulates the logic of processing mails.
        Right now, we are only receiving mails for the host which contain turn files.
        Should we do more things with mails, this would be the place to implement it. */
    class MailProcessor {
     public:
        /** Constructor.
            \param log  Logger
            \param mq   MailQueue interface (to send confirmation mails)
            \param host Host interface. We need multiple interfaces, hence this is a CommandHandler. */
        MailProcessor(afl::sys::LogListener& log,
                      server::interface::MailQueue& mq,
                      afl::net::CommandHandler& host);

        /** Process a mail message.
            \param mail MimeParser containing the pre-parsed message
            \retval true Message has been handled; user got a reply by mail
            \retval false Message could not be understood. Message should be saved for debugging. */
        bool process(const afl::net::MimeParser& mail);

     private:
        afl::sys::LogListener& m_log;
        server::interface::MailQueue& m_mailQueue;
        afl::net::CommandHandler& m_host;

        void logHeader(const afl::net::MimeParser& mail, const char* key);
        bool processPart(const afl::net::MimeParser& root, const afl::net::MimeParser& part, const String_t& address, String_t path);
        bool processTurnFile(const afl::net::MimeParser& root, const String_t& content, const String_t& address, String_t path);
        void sendRejection(const afl::net::MimeParser& root, const String_t& address, const String_t& path, const char* tpl);
    };

} }

#endif
