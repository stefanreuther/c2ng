/**
  *  \file client/dialogs/messagereceiver.hpp
  *  \brief Class client::dialogs::MessageReceiver
  */
#ifndef C2NG_CLIENT_DIALOGS_MESSAGERECEIVER_HPP
#define C2NG_CLIENT_DIALOGS_MESSAGERECEIVER_HPP

#include "afl/base/signalconnection.hpp"
#include "client/widgets/playersetselector.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"

namespace client { namespace dialogs {

    /** Message receiver dialog.
        Displays a PlayerSetSelector widget and buttons around it:
        - OK, Cancel
        - optional Help
        - optional Universal toggle
        - optional extra confirmation buttons (e.g. Revise)

        To use,
        - make a PlayerSetSelector
        - construct MessageReceiver
        - add extra features
        - call pack(), place on screen
        - call run() */
    class MessageReceiver : public ui::Window {
     public:
        /** Constructor.
            \param title   Dialog title
            \param sel     PlayerSetSelector
            \param root    UI root
            \param tx      Translator */
        MessageReceiver(String_t title, client::widgets::PlayerSetSelector& sel, ui::Root& root, afl::string::Translator& tx);

        /** Add "Universal" toggle.
            This button, when clicked, toggles the given set.
            \param set Set of all players
            \return *this */
        MessageReceiver& addUniversalToggle(game::PlayerSet_t set);

        /** Add extra confirmation button.
            This button, when clicked, exits the dialog with the given return value.
            \param label  Button key/label
            \param code   Desired return code
            \return *this */
        MessageReceiver& addExtra(util::KeyString label, int code);

        /** Add Help.
            \param helper Widget to invoke help
            \return *this
            \see ui::widgets::StandardDialogButtons::addHelp */
        MessageReceiver& addHelp(ui::Widget& helper);

        /** Run the dialog.
            \return return value: 0=cancel, 1=ok, other=values from addExtra() */
        int run();

     private:
        void onOK();
        void onToggleUniversal();
        void onSetChange();

        ui::Root& m_root;
        client::widgets::PlayerSetSelector& m_selector;
        afl::string::Translator& m_translator;

        afl::base::Deleter m_deleter;
        ui::EventLoop m_loop;

        ui::Group& m_actionGroup;
        ui::Widget& m_actionSpacer;
        ui::widgets::StandardDialogButtons& m_buttons;
        game::PlayerSet_t m_universalSet;

        afl::base::SignalConnection conn_setChange;
    };

} }

#endif
