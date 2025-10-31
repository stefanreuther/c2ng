/**
  *  \file client/dialogs/vcroptions.hpp
  *  \brief VCR Options Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_VCROPTIONS_HPP
#define C2NG_CLIENT_DIALOGS_VCROPTIONS_HPP

#include "afl/string/translator.hpp"
#include "client/vcr/configuration.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** VCR Options Dialog, edit-only version.
        Retrieves the options from the session, edits them, and writes them back.
        @param [in]      root       Root
        @param [in]      tx         Translator
        @param [in/out]  config     Configuration object to edit in-place
        @param [in]      pHelp      If non-null, widget to implement help
        @return true if dialog was confirmed, false if it was canceled (in this case, discard the @c config) */
    bool editVcrConfiguration(ui::Root& root, afl::string::Translator& tx, client::vcr::Configuration& config, ui::Widget* pHelp);

    /** VCR Options Dialog, main entry point.
        Retrieves the options from the session, edits them, and writes them back.
        @param root       Root
        @param tx         Translator
        @param gameSender Game sender */
    void editVcrOptions(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender);

} }

#endif
