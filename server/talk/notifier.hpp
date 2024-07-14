/**
  *  \file server/talk/notifier.hpp
  *  \brief Interface server::talk::Notifier
  */
#ifndef C2NG_SERVER_TALK_NOTIFIER_HPP
#define C2NG_SERVER_TALK_NOTIFIER_HPP

#include "afl/base/deletable.hpp"
#include "afl/data/stringlist.hpp"

namespace server { namespace talk {

    class Message;
    class UserPM;

    /** Interface for notifying messages.
        There methods are invoked to notify users of messages by sending email.
        They should eventually call the global notifyMessage() or notifyPM() functions. */
    class Notifier : public afl::base::Deletable {
     public:
        /** Trigger notification of a forum message.
            @param msg Message */
        virtual void notifyMessage(Message& msg) = 0;

        /** Trigger notification of a user PM.
            @param msg               Message
            @param notifyIndividual  Users that want an individual notification (with text)
            @param notifyGroup       Users that want a general notification (without text) */
        virtual void notifyPM(UserPM& msg, const afl::data::StringList_t& notifyIndividual, const afl::data::StringList_t& notifyGroup) = 0;
    };

} }

#endif
