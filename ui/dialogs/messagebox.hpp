/**
  *  \file ui/dialogs/messagebox.hpp
  */
#ifndef C2NG_UI_DIALOGS_MESSAGEBOX_HPP
#define C2NG_UI_DIALOGS_MESSAGEBOX_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/root.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/window.hpp"
#include "util/keystring.hpp"
#include "util/rich/text.hpp"

namespace ui { namespace dialogs {

    /** Standard message box.
        The message box contains
        - text (rich text with formatting and wird-wrapping permitted).
        - a number of buttons, typically "OK" or "Yes"/"No".

        To show a dialog box,
        - construct an object
        - call addButton() to add buttons, addKey() to add additional keys
        - call run() to operate the dialog box.
        addButton() and addKey() associate an integer code with the key/button which will be the return value of run() if that selection is chosen.

        As a shortcut, doYesNoDialog() and doOkDialog() perform a standard sequence of addButton() and run(). */
    class MessageBox : public Window {
     public:
        /** Constructor (plain-text content).
            \param text Message box content
            \param title Message box window title
            \param root UI root */
        MessageBox(String_t text, String_t title, Root& root);

        /** Constructor (rich-text content).
            \param text Message box content
            \param title Message box window title
            \param root UI root */
        MessageBox(util::rich::Text text, String_t title, Root& root);

        /** Constructor (arbitrary widget content).
            \param content Arbitrary widget. Must out-live the MessageBox.
            \param title Message box window title
            \param root UI root */
        MessageBox(Widget& content, String_t title, Root& root);

        /** Destructor. */
        ~MessageBox();

        /** Add a button.
            \param id Result identifier. This will be the return value of run() if this button is chosen.
            \param text Button text
            \param key Key
            \return *this */
        MessageBox& addButton(int id, String_t text, util::Key_t key);

        /** Add a button.
            \param id Result identifier. This will be the return value of run() if this button is chosen.
            \param ks KeyString defining both key and string
            \return *this */
        MessageBox& addButton(int id, const util::KeyString& ks);

        /** Add a key.
            \param id Result identifier. This will be the return value of run() if this button is chosen.
            \param key Key
            \return *this */
        MessageBox& addKey(int id, util::Key_t key);

        /** Add a help button.
            \param helper Widget to receive the button's keypress
            \param tx Translator */
        MessageBox& addHelp(Widget& helper, afl::string::Translator& tx);

        /** Ignore a key.
            By default, MessageBox auto-connects Key_Return and Key_Escape to the first and last button, respectively, unless you map them to a different action.
            Use this call to disable them entirely without mapping them to any action.
            \param key Key to ignore
            \return *this */
        MessageBox& ignoreKey(util::Key_t key);

        /** Operate the dialog.
            Displays the dialog and returns the result identifier of the user's chosen selection.

            If you did not define a mapping for Return and/or Escape, those keys will select the first/last button, respectively.

            You must not call addButton(), addKey() after calling run. */
        int run();

        /** Build and operate a Yes/No dialog.
            You should not have called addButton(), addKey() on this object yet.
            \param tx Translator
            \return user's selection (true=yes, false=no) */
        bool doYesNoDialog(afl::string::Translator& tx);

        /** Build and operate a simple confirmation dialog (just an OK button).
            You should not have called addButton(), addKey() on this object yet.
            \param tx Translator */
        void doOkDialog(afl::string::Translator& tx);

     private:
        void init(const util::rich::Text& text);
        void init(Widget& content);
        void checkKey(int id, util::Key_t key, bool isButton);

        afl::base::Deleter m_deleter;
        Group& m_buttonGroup;
        ui::widgets::KeyDispatcher& m_keyDispatcher;
        Root& m_root;
        EventLoop m_loop;

        enum Flag {
            HaveReturn,
            HaveEscape,
            HaveFirst,
            HaveRun
        };
        afl::bits::SmallSet<Flag> m_flags;
        int m_firstCommand;
        int m_lastCommand;
    };

} }

#endif
