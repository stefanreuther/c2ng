/**
  *  \file client/dialogs/techupgradedialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_TECHUPGRADEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_TECHUPGRADEDIALOG_HPP

#include "ui/root.hpp"
#include "game/types.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace dialogs {

    void doTechUpgradeDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t pid);

} }

#endif
