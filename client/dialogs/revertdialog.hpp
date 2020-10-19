/**
  *  \file client/dialogs/revertdialog.hpp
  *  \brief "Reset Location" dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_REVERTDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_REVERTDIALOG_HPP

#include "game/map/point.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "afl/string/translator.hpp"

namespace client { namespace dialogs {

    /** "Reset Location" dialog.
        \param root        UI Root
        \param gameSender  Sender to communicate with game session
        \param tx          Translator
        \param pos         Location to reset */
    void doRevertLocation(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, game::map::Point pos);

} }

#endif
