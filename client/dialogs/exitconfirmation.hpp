/**
  *  \file client/dialogs/exitconfirmation.hpp
  *  \brief Exit Confirmation Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_EXITCONFIRMATION_HPP
#define C2NG_CLIENT_DIALOGS_EXITCONFIRMATION_HPP

#include "afl/string/translator.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    const int ExitDialog_Save = 1;     ///< Save game (if disabled, don't save).
    const int ExitDialog_Exit = 2;     ///< Exit (if disabled, keep playing).

    /** Ask for exit confirmation.
        @param root  UI root
        @param tx    Translator
        @return Selected options (combination of ExitDialog_Save, ExitDialog_Exit) */
    int askExitConfirmation(ui::Root& root, afl::string::Translator& tx);

} }

#endif
