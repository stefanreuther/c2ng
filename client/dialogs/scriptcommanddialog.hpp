/**
  *  \file client/dialogs/scriptcommanddialog.hpp
  *  \brief Script Command Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_SCRIPTCOMMANDDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SCRIPTCOMMANDDIALOG_HPP

#include "client/si/userside.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/eventloop.hpp"
#include "ui/widgets/inputline.hpp"

namespace client { namespace dialogs {

    /** Script command dialog.
        Allows the user to enter a script command, with optional completion. */
    class ScriptCommandDialog : private gfx::KeyEventConsumer {
     public:
        /** Constructor.
            @param prompt      Prompt
            @param userSide    UserSide (for UI Root, Translator, game sender, ContextProvider) */
        ScriptCommandDialog(String_t prompt, client::si::UserSide& userSide);

        /** Set command.
            Predefines the content of the input field.
            @param cmd Command */
        void setCommand(String_t cmd);

        /** Get command.
            @return user input */
        String_t getCommand() const;

        /** Set help page name.
            If given and non-empty, the dialog will have a "Help" button.
            @param help Help page name */
        void setHelp(String_t help);

        /** Set title.
            @param title Title. Default is same as prompt. */
        void setTitle(String_t title);

        /** Set whether to complete only commands.
            @param onlyCommands true to complete only command verbs, false (default) to also complete variables, config, etc.
            @see game::interface::buildCompletionList() */
        void setOnlyCommands(bool onlyCommands);

        /** Set whether to enforce auto-tasks.
            @param enforceTask true to only accept commands that satisfy the interpreter::TaskEditor::isValidCommand() check */
        void setEnforceTask(bool enforceTask);

        /** Operate the dialog.
            @retval true   User confirmed valid input
            @retval false  User canceled */
        bool run();

     private:
        String_t m_prompt;
        String_t m_title;
        String_t m_help;
        client::si::UserSide& m_userSide;
        bool m_onlyCommands;
        bool m_enforceTask;

        ui::widgets::InputLine m_input;
        ui::EventLoop m_loop;

        // KeyEventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix);

        // Other event handlers:
        void onOK();
    };


    /** Perform command completion for an input line.
        Retrieves completion from game side for the current context as reported by the UserSide,
        executes possibly-needed interaction, and inserts the result into the given input line.

        @param input        Input line widget
        @param userSide     UserSide (for game sender, translator, root)
        @param onlyCommands true to complete only command verbs, false to also complete variables, config, etc.
        @see game::interface::buildCompletionList
        @see game::proxy::ScriptEditorProxy::buildCompletionList */
    void doCompletion(ui::widgets::InputLine& input, client::si::UserSide& userSide, bool onlyCommands);

} }

#endif
