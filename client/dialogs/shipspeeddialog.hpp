/**
  *  \file client/dialogs/shipspeeddialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_SHIPSPEEDDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SHIPSPEEDDIALOG_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    void doShipSpeedDialog(game::Id_t shipId,
                           ui::Root& root,
                           afl::string::Translator& tx,
                           util::RequestSender<game::Session> gameSender);

} }

#endif
