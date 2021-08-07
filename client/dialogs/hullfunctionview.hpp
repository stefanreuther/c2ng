/**
  *  \file client/dialogs/hullfunctionview.hpp
  *  \brief Hull function detail view dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_HULLFUNCTIONVIEW_HPP
#define C2NG_CLIENT_DIALOGS_HULLFUNCTIONVIEW_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/spec/info/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Show hull function details.
        \param content      Data to show
        \param root         UI root
        \param gameSender   Game sender (for help)
        \param tx           Translator */
    void showHullFunctions(const game::spec::info::AbilityDetails_t& content, ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

} }

#endif
