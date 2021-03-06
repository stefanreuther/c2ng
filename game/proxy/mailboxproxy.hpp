/**
  *  \file game/proxy/mailboxproxy.hpp
  *  \brief Class game::proxy::MailboxProxy
  */
#ifndef C2NG_GAME_PROXY_MAILBOXPROXY_HPP
#define C2NG_GAME_PROXY_MAILBOXPROXY_HPP

#include "game/msg/browser.hpp"
#include "game/msg/mailbox.hpp"
#include "game/proxy/mailboxadaptor.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/rich/text.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Bidirectional proxy for message access.
        Proxies access to a game::msg::Mailbox.

        The game::msg::Mailbox object is selected using a MailboxAdaptor instance provided by the caller.
        That adaptor also provides a few surrounding objects, as well as the ability to store a current position.

        Synchronous, bidirectional:
        - query position and count

        Asynchronous, bidirectional:
        - request one message's information and details

        For now, this needs an initial call to setCurrentMessage() to retrieve the first message's information.
        Every call to a message selection function (setCurrentMessage(), browse()) will be followed by a sig_update event,
        although sequences of multiple calls may produce only one sig_update in total. */
    class MailboxProxy {
     public:
        /** Status. */
        struct Status {
            size_t numMessages;              ///< Number of messages. \see game::msg::Mailbox::getNumMessages().
            size_t currentMessage;           ///< Current message. \see MailboxAdaptor::getCurrentMessage().
            Status()
                : numMessages(0), currentMessage(0)
                { }
        };

        /** Message information. */
        struct Message {
            util::rich::Text text;                   ///< Message text, formatted with links. See game::msg::formatMessage.
            bool isFiltered;                         ///< true if message is filtered (hidden by default).
            game::Reference goto1;                   ///< First associated object, if any.
            String_t goto1Name;                      ///< First associated object name, if any.
            game::Reference goto2;                   ///< Second associated object, if any.
            String_t goto2Name;                      ///< Second associated object name, if any.
            game::PlayerSet_t reply;                 ///< Players to send reply to, if any.
            game::PlayerSet_t replyAll;              ///< Players to send "reply all" to, if any.
            String_t replyName;                      ///< Name of player to send reply to, if any.
            game::msg::Mailbox::Flags_t flags;       ///< Flags.
            game::msg::Mailbox::Actions_t actions;   ///< Actions.
            Id_t id;                                 ///< Message Id (for outgoing messages).

            Message()
                : text(), isFiltered(false), goto1(), goto1Name(), goto2(), goto2Name(), reply(), replyAll(), replyName(), flags(), actions(), id()
                { }
        };


        /** Constructor.
            \param sender     Sender
            \param receiver   RequestDispatcher to receive updates in this thread */
        MailboxProxy(util::RequestSender<MailboxAdaptor> sender, util::RequestDispatcher& recv);

        /** Destructor. */
        ~MailboxProxy();

        /** Get current status.
            Retrieves number of messages and last position.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] status Status */
        void getStatus(WaitIndicator& ind, Status& status);

        /** Get summary.
            Retrieves a list of subject lines.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] summary Summary list
            \param [out] index   Index into summary list of current message */
        void getSummary(WaitIndicator& ind, game::msg::Browser::Summary_t& summary, size_t& index);

        /** Set current message.
            Eventually replies with sig_update.
            Valid indexes are [0, Status::numMessages).
            \param index New index */
        void setCurrentMessage(size_t index);

        /** Browse messages.
            Eventually replies with sig_update.
            \param mode            Browsing mode
            \param amount          Number of steps to go
            \param acceptFiltered  true to accept filtered messages, false to skip
            \see game::msg::Browser::browse */
        void browse(game::msg::Browser::Mode mode, int amount, bool acceptFiltered);

        /** Toggle whether heading is filtered.
            Responds with sig_summaryChanged.
            \param heading Heading
            \see game::msg::Configuration::toggleHeadingFiltered */
        void toggleHeadingFiltered(String_t heading);

        /** Perform action on current message.
            \param a Action
            \see game::msg::Mailbox::performMessageAction */
        void performMessageAction(game::msg::Mailbox::Action a);

        /** Signal: message update.
            \param msg Message */
        afl::base::Signal<void(size_t index, const Message&)> sig_update;

        /** Signal: summary changed.
            \param summary Updated summary */
        afl::base::Signal<void(const game::msg::Browser::Summary_t&)> sig_summaryChanged;

     private:
        class Trampoline;
        class TrampolineFromAdaptor;

        util::RequestReceiver<MailboxProxy> m_reply;
        util::RequestSender<Trampoline> m_request;

        size_t m_numRequests;

        void updateCurrentMessage(size_t index, Message data, bool requested);
    };

} }

#endif
