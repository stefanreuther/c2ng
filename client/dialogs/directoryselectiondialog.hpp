/**
  *  \file client/dialogs/directoryselectiondialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_DIRECTORYSELECTIONDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_DIRECTORYSELECTIONDIALOG_HPP

#include "ui/root.hpp"
#include "afl/string/string.hpp"
#include "game/browser/session.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    bool doDirectorySelectionDialog(ui::Root& root, util::RequestSender<game::browser::Session> session, String_t& folderName);

} }

#endif
