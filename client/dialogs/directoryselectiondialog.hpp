/**
  *  \file client/dialogs/directoryselectiondialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_DIRECTORYSELECTIONDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_DIRECTORYSELECTIONDIALOG_HPP

#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/browser/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    bool doDirectorySelectionDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::browser::Session> session, String_t& folderName);

} }

#endif
