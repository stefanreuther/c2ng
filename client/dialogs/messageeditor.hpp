/**
  *  \file client/dialogs/messageeditor.hpp
  *  \brief Class client::dialogs::MessageEditor
  */
#ifndef C2NG_CLIENT_DIALOGS_MESSAGEEDITOR_HPP
#define C2NG_CLIENT_DIALOGS_MESSAGEEDITOR_HPP

#include "game/proxy/outboxproxy.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/widgets/editor.hpp"
#include "util/editor/editor.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Message editor dialog.
        Displays a dialog to edit a message: user can edit the text and modify the receiver.

        Usage:
        - construct
        - use setReceivers(), setSender() (mandatory), setTitle(), setText() (optional) to set initial content
        - call run()
        - if run() returns nonzero, update/create the message according to getText(), getReceivers(), getSender() */
    class MessageEditor {
     public:
        /** Constructor.
            \param root        UI root
            \param proxy       OutboxProxy (used for building headers, createStringVerifier)
            \param gameSender  Game sender (used for PlayerProxy, help)
            \param tx          Translator */
        MessageEditor(ui::Root& root, game::proxy::OutboxProxy& proxy, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);
        ~MessageEditor();

        /** Set dialog title.
            Call before run().
            \param title Dialog title */
        void setTitle(String_t title);

        /** Set message text.
            Call before run().
            \param text Message text (multi-line string) */
        void setText(String_t text);

        /** Set receiver set.
            Call before run().
            \param receivers Receivers */
        void setReceivers(game::PlayerSet_t receivers);

        /** Set message sender (viewpoint player).
            Call before run().
            \param sender Message sender */
        void setSender(int sender);

        /** Get message text.
            \return text (multi-line string) */
        String_t getText() const;

        /** Get receiver set.
            \return set */
        game::PlayerSet_t getReceivers() const;

        /** Get message sender.
            \return sender */
        int getSender() const;

        /** Run dialog.
            \return true: user confirmed; false: user canceled */
        bool run();

     private:
        void onChangeReceivers();
        void onCancel();
        void onSend();
        void updateContent(game::proxy::WaitIndicator& ind);

        ui::Root& m_root;
        game::proxy::OutboxProxy& m_proxy;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        String_t m_title;
        util::editor::Editor m_editor;
        ui::EventLoop m_loop;
        game::PlayerSet_t m_receivers;
        int m_sender;

        size_t m_numHeaderLines;
    };

} }

#endif
