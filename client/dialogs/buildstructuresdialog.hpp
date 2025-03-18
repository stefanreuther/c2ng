/**
  *  \file client/dialogs/buildstructuresdialog.hpp
  *  \brief Structure Building Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDSTRUCTURESDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_BUILDSTRUCTURESDIALOG_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Structure Building Dialog (BuildStructuresProxy, TaxationProxy).
        Creates transactions, operates the dialog, and commits the transactions if chosen.
        @param root        Root
        @param gameSender  Game sender
        @param tx          Translator
        @param pid         Planet to work in
        @param page        Initial page to show [0, 2] */
    void doBuildStructuresDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, game::Id_t pid, int page);

} }

#endif
