/**
  *  \file client/dialogs/keymapdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_KEYMAPDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_KEYMAPDIALOG_HPP

#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace dialogs {

    void doKeymapDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, String_t keymapName);

} }

#endif
