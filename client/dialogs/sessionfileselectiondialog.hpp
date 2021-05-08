/**
  *  \file client/dialogs/sessionfileselectiondialog.hpp
  *  \brief Class client::dialogs::SessionFileSelectionDialog
  */
#ifndef C2NG_CLIENT_DIALOGS_SESSIONFILESELECTIONDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SESSIONFILESELECTIONDIALOG_HPP

#include "client/dialogs/fileselectiondialog.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"

namespace client { namespace dialogs {

    /** FileSelectionDialog with a game::Session.
        FileSelectionDialog is generic by only requiring a afl::io::FileSystem.
        This class extends it to add the necessary boilerplate to run on a game::Session and integrate with it.
        - conversion of the RequestSender
        - management of the UI.DIRECTORY variable

        To use,
        - construct
        - configure as usual
        - use runDefault() to run with default integration
          (but using the normal run() to use only the parts you're interested in is supported). */
    class SessionFileSelectionDialog : public FileSelectionDialog {
     public:
        /** Constructor.
            \param root        Root
            \param tx          Translator
            \param gameSender  Access to game::Session
            \param title       Window title */
        SessionFileSelectionDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, String_t title);
        ~SessionFileSelectionDialog();

        /** Set folder from the session's UI.DIRECTORY.
            Call before run().
            \param ind WaitIndicator for UI synchronisation */
        void setFolderFromSession(game::proxy::WaitIndicator& ind);

        /** Store current folder in the session's UI.DIRECTORY.
            Call after run(). */
        void storeFolderInSession();

        /** Run dialog with default integration.
            Currently (20201226) this is the setFolderFromSession/storeFolderInSession sequence around run().
            \param ind WaitIndicator for UI synchronisation
            \return true if user chose OK, false on cancel
            \see FileSelectionDialog::run */
        bool runDefault(game::proxy::WaitIndicator& ind);

     private:
        util::RequestSender<game::Session> m_gameSender;
    };

} }

#endif
