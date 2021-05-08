/**
  *  \file client/dialogs/teamsettings.hpp
  *  \brief Team editor dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_TEAMSETTINGS_HPP
#define C2NG_CLIENT_DIALOGS_TEAMSETTINGS_HPP

#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace dialogs {

    /** Team editor dialog.
        \param root       Root
        \param gameSender Game sender
        \param tx         Translator */
    void editTeams(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

} }

#endif
