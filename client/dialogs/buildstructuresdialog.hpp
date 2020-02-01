/**
  *  \file client/dialogs/buildstructuresdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDSTRUCTURESDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_BUILDSTRUCTURESDIALOG_HPP

#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "afl/string/translator.hpp"

namespace client { namespace dialogs {

    void doBuildStructuresDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, game::Id_t pid, int page);

} }

#endif
