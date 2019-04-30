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
        /** Status of a user's email address.
            Note that (as with all enums) it can happen that values outside this range are produced. */
        enum AddressStatus {
            NotSet      = 0,
            Requested   = 'r',
            Confirmed   = 'c',
            Unconfirmed = 'u',
            Blocked     = 'b'
        };

        /** Status of a user. */
        struct UserStatus {
            /** Email address. Can be empty. */
            String_t address;
            /** Status. */
            AddressStatus status;

            UserStatus()
                : address(), status(NotSet)
                { }
        };

        /** Start sending a mail (MAIL tpl:Str, [uniq:Str]).
            Call addParameter()/addAttachment() next, then send().
            \param templateName Name of template
            \param uniqueId Unique identifier of this mail. Cancel a previous mail with that identifier. */
        virtual void startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId) = 0;

        /** Set parameter (PARAM name:Str, val:Str).
            \param name Parameter name
            \param val Value */
        virtual void addParameter(String_t parameterName, String_t value) = 0;

        /** Add attachment (ATTACH url:Str).
            \param url URL */
        virtual void addAttachment(String_t url) = 0;

        /** Send prepared email (SEND addr:Str...).
            \param receivers List of receivers */
        virtual void send(afl::base::Memory<const String_t> receivers) = 0;

        /** Cancel a queued email (CANCEL uniq:Str).
            \param uniqueId Identifier used with startMessage() */
        virtual void cancelMessage(String_t uniqueId) = 0;

        /** Confirm email address (CONFIRM addr:Str, key:Str, [info:Str]).
            Throws if the address/key do not match.
            \param address Address
            \param key Key
            \param info Optional info for abuse tracking */
        virtual void confirmAddress(String_t address, String_t key, afl::base::Optional<String_t> info) = 0;

        /** Request confirmation for a user (REQUEST user:UID).
            \param user User Id */
        virtual void requestAddress(String_t user) = 0;

        /** Process queue (RUNQUEUE). */
        virtual void runQueue() = 0;

        /** Get user's email status (STATUS user:Str).
            \param user User Id */
        virtual UserStatus getUserStatus(String_t user) = 0;


        /** Convert AddressStatus into a string.
            \param st Status */
        static String_t formatAddressStatus(AddressStatus st);

        /** Parse string into AddressStatus.
            \param st Status */
        static AddressStatus parseAddressStatus(const String_t& st);
    };

} }

#endif
