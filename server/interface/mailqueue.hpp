/**
  *  \file server/interface/mailqueue.hpp
  *  \brief Interface server::interface::MailQueue
  */
#ifndef C2NG_SERVER_INTERFACE_MAILQUEUE_HPP
#define C2NG_SERVER_INTERFACE_MAILQUEUE_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"
#include "afl/base/memory.hpp"

namespace server { namespace interface {

    /** Interface to Mail Queue service (mailout). */
    class MailQueue : public afl::base::Deletable {
     public:
        // RESP Syntax: MAIL tpl:Str, [uniq:Str]
        virtual void startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId) = 0;

        // PARAM name:Str, val:Str
        virtual void addParameter(String_t parameterName, String_t value) = 0;

        // ATTACH url:Str
        virtual void addAttachment(String_t url) = 0;

        // SEND addr:Str...
        virtual void send(afl::base::Memory<const String_t> receivers) = 0;

        // CANCEL uniq:Str
        virtual void cancelMessage(String_t uniqueId) = 0;

        // CONFIRM addr:Str, key:Str, [info:Str]
        virtual void confirmAddress(String_t address, String_t key, afl::base::Optional<String_t> info) = 0;

        // REQUEST user:UID
        virtual void requestAddress(String_t user) = 0;

        // RUNQUEUE
        virtual void runQueue() = 0;
    };

} }

#endif
