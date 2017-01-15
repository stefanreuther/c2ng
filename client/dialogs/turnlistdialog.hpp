/**
  *  \file client/dialogs/turnlistdialog.hpp
  *  \brief Class client::dialogs::TurnListDialog
  */
#ifndef C2NG_CLIENT_DIALOGS_TURNLISTDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_TURNLISTDIALOG_HPP

#include "afl/string/translator.hpp"
#include "client/widgets/turnlistbox.hpp"
#include "game/session.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Turn list (history) dialog.
        Displays a list of turns and lets the user choose one.
        Controls both loading of status information and turn data. */
    class TurnListDialog {
     public:
        /** Constructor.
            \param root UI root
            \param tx Translator
            \param sender Access to game session
            \param initialDelta Initial cursor adjustment: 0=place on current turn, -1=place on previous turn */
        TurnListDialog(ui::Root& root,
                       afl::string::Translator& tx,
                       util::RequestSender<game::Session> sender,
                       int initialDelta);

        /** Destructor. */
        ~TurnListDialog();

        /** Execute the dialog.
            \return Chosen turn number; 0 if dialog cancelled. */
        int run();

        /** Callback: initial dialog setup.
            \param content [in/out] data for all turns; can be destroyed.
            \param turnNumber [in] currently displayed turn number */
        void handleSetup(std::vector<client::widgets::TurnListbox::Item>& content, int turnNumber);

        /** Callback: partial data update.
            \param content [in/out] data to update; can be destroyed. An empty vector means there was a problem with updating data. */
        void handleUpdate(std::vector<client::widgets::TurnListbox::Item>& content);

     private:
        /** Status of communication with game session. */
        enum State {
            LoadingInitial,            ///< Loading initial content; expecting handleSetup() callback.
            LoadingStatus,             ///< Loading status of some turns; expecting handleUpdate() callback.
            LoadingTurn,               ///< Loading a turn; expecting handleUpdate() callback.
            NoMoreWork                 ///< Not doing anything.
        };
        State m_state;
        const int m_initialDelta;

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::Session> m_sender;
        util::RequestReceiver<TurnListDialog> m_receiver;

        client::widgets::TurnListbox m_list;
        ui::EventLoop m_loop;

        afl::base::Ref<gfx::Timer> m_activationTimer;
        bool m_pendingActivation;

        void onOK();
        void onCancel();
        void onScroll();
        void onActivationTimer();

        bool handleSelect();

        void postNextRequest(bool allowUpdate);
    };

} }

#endif
