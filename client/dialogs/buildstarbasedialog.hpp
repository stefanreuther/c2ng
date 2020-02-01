/**
  *  \file client/dialogs/buildstarbasedialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDSTARBASEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_BUILDSTARBASEDIALOG_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    void doBuildStarbaseDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, game::Id_t pid);

} }

#endif
