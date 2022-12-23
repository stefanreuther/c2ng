/**
  *  \file game/proxy/outboxproxy.hpp
  *  \brief Class game::proxy::OutboxProxy
  */
#ifndef C2NG_GAME_PROXY_OUTBOXPROXY_HPP
#define C2NG_GAME_PROXY_OUTBOXPROXY_HPP

#include "game/session.hpp"
#include "game/proxy/mailboxadaptor.hpp"
#include "util/requestsender.hpp"
#include "game/stringverifier.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Bidirectional proxy for sent-messages access.
        Proxies access to a game::msg::Outbox.
        OutboxProxy only offers outbox-specific operations.
        For general mailbox access, use MailboxProxy; the required MailboxAdaptor can be created using getMailboxAdaptor().

        For OutboxProxy, messages are addressed using Ids, not indexes.
        Ids remain stable when messages are modified in the background.
        An attempt to modify a message with an unknown Id is silently ignored.

        Synchronous, bidirectional:
        - get information

        Asynchronous:
        - send (create) message
        - modify message
        - delete message */
    class OutboxProxy {
     public:
        /** Information about a message. */
        struct Info {
            PlayerSet_t receivers;      ///< Receiver set.
            String_t text;              ///< Message text (editable part).
            int sender;                 ///< Message sender.
            Info()
                : receivers(), text(), sender()
                { }
        };

        /** Constructor.
            \param sender Game sender */
        explicit OutboxProxy(util::RequestSender<Session> sender);
        ~OutboxProxy();

        /** Get message headers for display.
            \param [in,out] ind       UI synchronisation
            \param [in]     sender    Message sender
            \param [in]     receivers Message receivers
            \return headers (multi-line string, ending in "\n")
            \see game::msg::Outbox::getHeadersForDisplay() */
        String_t getHeadersForDisplay(WaitIndicator& ind, int sender, PlayerSet_t receivers);

        /** Get message data.
            \param [in,out] ind       UI synchronisation
            \param [in]     id        Message Id
            \param [out]    result    Message information
            \return true Success */
        bool getMessage(WaitIndicator& ind, Id_t id, Info& result);

        /** Get string verifier.
            Obtains a clone of the game's StringVerifier.
            \param [in,out] ind       UI synchronisation
            \return Newly-allocated StringVerifier. Caller takes ownership. Can be null. */
        StringVerifier* createStringVerifier(WaitIndicator& ind);

        /** Add message.
            The message is created asynchronously.
            \param [in]    sender     Message sender
            \param [in]    text       Message text
            \param [in]    receivers  Message receivers
            \see game::msg::Outbox::addMessage */
        void addMessage(int sender, String_t text, PlayerSet_t receivers);

        /** Set message text.
            The message is updated asynchronously.
            \param [in] id    Id
            \param [in] text  Message text */
        void setMessageText(Id_t id, String_t text);

        /** Set message receivers.
            The message is updated asynchronously.
            \param [in] id         Id
            \param [in] receivers  Message receivers */
        void setMessageReceivers(Id_t id, PlayerSet_t receivers);

        /** Delete message.
            The message is deleted asynchronously.
            \param [in] id  Id */
        void deleteMessage(Id_t id);

        /** Store message to file.
            \param [in,out] ind      UI synchronisation
            \param [in]     sender   Message sender
            \param [in]     text     Message text
            \param [in]     fileName File name
            \param [out]    errorMessage Error message
            \return true on success, false on error (errorMessage set) */
        bool addMessageToFile(WaitIndicator& ind, int sender, String_t text, String_t fileName, String_t& errorMessage);

        /** Load message text from file.
            \param [in,out] ind      UI synchronisation
            \param [out]    text     Message text
            \param [in]     fileName File name
            \param [out]    errorMessage Error message
            \return true on success, false on error (errorMessage set) */
        bool loadMessageTextFromFile(WaitIndicator& ind, String_t& text, String_t fileName, String_t& errorMessage);

        /** Get MailboxAdaptor.
            \return MailboxAdaptor that accesses the same Outbox this proxy is accessing */
        util::RequestSender<MailboxAdaptor> getMailboxAdaptor();

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
