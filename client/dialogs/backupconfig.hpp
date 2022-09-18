/**
  *  \file client/dialogs/backupconfig.hpp
  *  \brief Backup Configuration Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_BACKUPCONFIG_HPP
#define C2NG_CLIENT_DIALOGS_BACKUPCONFIG_HPP

#include "ui/root.hpp"
#include "afl/string/translator.hpp"
#include "afl/string/string.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace dialogs {

    /** Backup Configuration Dialog.
        Allows the user to enter a backup configuration path, with extra UI to select a default path.
        @param [in,out] value         Current value
        @param [in]     defaultValue  Default value
        @param [in]     root          UI root
        @param [in]     gameSender    Game sender (for help)
        @param [in]     tx            Translator
        @return true if user entered a new name (@c value updated); false if user canceled */
    bool editBackupConfiguration(String_t& value,
                                 const String_t& defaultValue,
                                 ui::Root& root,
                                 util::RequestSender<game::Session> gameSender,
                                 afl::string::Translator& tx);

} }

#endif
