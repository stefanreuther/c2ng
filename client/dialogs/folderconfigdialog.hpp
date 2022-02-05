/**
  *  \file client/dialogs/folderconfigdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_FOLDERCONFIGDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_FOLDERCONFIGDIALOG_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/browserproxy.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Folder configuration dialog.
        This dialog allows configuring per-folder options (=game::Root::aConfigureXXX).

        \param root Root
        \param proxy Proxy to communicate with browser
        \param tx Translator */
    void doFolderConfigDialog(ui::Root& root, game::proxy::BrowserProxy& proxy, afl::string::Translator& tx);

} }

#endif
