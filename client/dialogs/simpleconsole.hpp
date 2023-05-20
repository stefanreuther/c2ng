/**
  *  \file client/dialogs/simpleconsole.hpp
  *  \brief Class client::dialogs::SimpleConsole
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMPLECONSOLE_HPP
#define C2NG_CLIENT_DIALOGS_SIMPLECONSOLE_HPP

#include "afl/string/translator.hpp"
#include "client/widgets/consolecontroller.hpp"
#include "client/widgets/consoleview.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/widgets/button.hpp"

namespace client { namespace dialogs {

    /** Simple console window.
        Displays a ConsoleView, and receives messages from an ongoing operation.
        Allows the user to read and scroll through the messages as they arrive.
        When the operation finished, user can confirm the dialog.

        Usage:
        - create
        - call run()

        From callbacks (must be from UI thread), call addMessage() to add messages;
        call enableClose() to enable the "close" button and allow the user to confirm the dialog.
        When the user confirms, run() returns. */
    class SimpleConsole {
     public:
        /** Constructor.
            @param root UI root
            @param tx   Translator
            @param numLines Number of lines to show */
        SimpleConsole(ui::Root& root, afl::string::Translator& tx, int numLines);

        /** Add a message.
            The message will be added as plain text.
            @param str Text */
        void addMessage(String_t str);

        /** Enable "close" button. */
        void enableClose();

        /** Show the dialog.
            This function returns when the user confirms the "close" button.
            @param title Window title */
        void run(String_t title);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        client::widgets::ConsoleView m_consoleView;
        client::widgets::ConsoleController m_consoleController;
        ui::widgets::Button m_closeButton;
        ui::EventLoop m_loop;
        bool m_allowClose;

        void onClose();
        void updateClose();
    };

} }

#endif
