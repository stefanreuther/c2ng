/**
  *  \file client/dialogs/scores.hpp
  *  \brief Score Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_SCORES_HPP
#define C2NG_CLIENT_DIALOGS_SCORES_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    void showScores(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

} }

#endif
