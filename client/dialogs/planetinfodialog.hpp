/**
  *  \file client/dialogs/planetinfodialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_PLANETINFODIALOG_HPP
#define C2NG_CLIENT_DIALOGS_PLANETINFODIALOG_HPP

#include "afl/string/translator.hpp"
#include "game/map/point.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    void doPlanetInfoDialog(ui::Root& root,
                            util::RequestSender<game::Session> gameSender,
                            game::Id_t planetId,
                            afl::string::Translator& tx);

    void doPlanetInfoDialog(ui::Root& root,
                            util::RequestSender<game::Session> gameSender,
                            game::map::Point pos,
                            afl::string::Translator& tx);

} }

#endif
