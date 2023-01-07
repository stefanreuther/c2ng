/**
  *  \file client/dialogs/directorysetup.hpp
  *  \brief Game Directory Setup dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_DIRECTORYSETUP_HPP
#define C2NG_CLIENT_DIALOGS_DIRECTORYSETUP_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/browserproxy.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"

namespace client { namespace dialogs {

    /** Game Directory Setup dialog.
        Asks the user for a game directory for the current game, and configures that on the BrowserProxy.

        @param proxy BrowserProxy instance
        @param pHelp Help widget (optional)
        @param root  UI root
        @param tx    Translator
        @return true on success, false if dialog was canceled */
    bool doDirectorySetupDialog(game::proxy::BrowserProxy& proxy, ui::Widget* pHelp, ui::Root& root, afl::string::Translator& tx);

} }

#endif
