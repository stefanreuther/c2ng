/**
  *  \file client/dialogs/screenhistorydialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_SCREENHISTORYDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SCREENHISTORYDIALOG_HPP

#include "client/screenhistory.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace dialogs {

    int32_t doScreenHistoryDialog(ui::Root& root,
                                  util::RequestSender<game::Session> gameSender,
                                  ScreenHistory& history,
                                  bool excludeCurrent);

} }

#endif
