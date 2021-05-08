/**
  *  \file game/proxy/mailboxadaptor.hpp
  *  \brief Interface game::proxy::MailboxAdaptor
  */
#ifndef C2NG_GAME_PROXY_MAILBOXADAPTOR_HPP
#define C2NG_GAME_PROXY_MAILBOXADAPTOR_HPP

#include "afl/base/deletable.hpp"
#include "game/msg/mailbox.hpp"
#include "game/session.hpp"

namespace game { namespace proxy {

    /** Adaptor to access a mailbox and environment. */
    class MailboxAdaptor : public afl::base::Deletable {
     public:
        /** Access session.
            Required for
            - Translator
            - Root (contains player names)
            - Game (contains configuration)
            \return session */
        virtual Session& session() const = 0;

        /** Access mailbox.
            \return mailbox */
        virtual game::msg::Mailbox& mailbox() const = 0;

        /** Get configuration.
            Can return null if this mailbox shall not use message configuration.
            \return configuration */
        virtual game::msg::Configuration* getConfiguration() const = 0;

        /** Get index of last viewed message.
            If you don't persist that status, return 0.
            \return index */
        virtual size_t getCurrentMessage() const = 0;

        /** Set current message index.
            Store the index that the next getCurrentMessage() (in a new Adaptor instance, maybe) can find it.
            If you don't persist that status, ignore the call.
            \param n Index */
        virtual void setCurrentMessage(size_t n) = 0;
    };

} }

#endif
