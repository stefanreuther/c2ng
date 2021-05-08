/**
  *  \file client/dialogs/helpdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_HELPDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_HELPDIALOG_HPP

#include "afl/string/string.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    void doHelpDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, String_t pageName);

} }

#endif
