/**
  *  \file client/dialogs/folderconfigdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_FOLDERCONFIGDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_FOLDERCONFIGDIALOG_HPP

#include "game/browser/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "afl/string/string.hpp"

namespace client { namespace dialogs {

    /** Folder configuration dialog.
        This dialog allows configuring per-folder options (=game::Root::aConfigureXXX).

        \param root Root
        \param sender Sender to communicate with browser
        \param tx Translator */
    void doFolderConfigDialog(ui::Root& root,
                              util::RequestSender<game::browser::Session> sender,
                              afl::string::Translator& tx);

} }

#endif
