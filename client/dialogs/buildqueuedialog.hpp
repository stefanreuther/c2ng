/**
  *  \file client/dialogs/buildqueuedialog.hpp
  *  \brief Build Queue Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDQUEUEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_BUILDQUEUEDIALOG_HPP

#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "afl/string/translator.hpp"

namespace client { namespace dialogs {

    void doBuildQueueDialog(game::Id_t baseId,
                            ui::Root& root,
                            util::RequestSender<game::Session> gameSender,
                            afl::string::Translator& tx);

} }

#endif
