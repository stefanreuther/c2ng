/**
  *  \file client/dialogs/buildqueuedialog.hpp
  *  \brief Build Queue Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDQUEUEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_BUILDQUEUEDIALOG_HPP

#include "afl/string/translator.hpp"
#include "client/screenhistory.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Show build queue dialog.
        \param baseId      Invoking base Id
        \param root        UI root
        \param gameSender  Game sender (for proxies)
        \param tx          Translator
        \return Screen to activate */
    ScreenHistory::Reference doBuildQueueDialog(game::Id_t baseId,
                                                ui::Root& root,
                                                util::RequestSender<game::Session> gameSender,
                                                afl::string::Translator& tx);

} }

#endif
