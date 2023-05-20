/**
  *  \file client/dialogs/unpack.hpp
  *  \brief Unpack dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_UNPACK_HPP
#define C2NG_CLIENT_DIALOGS_UNPACK_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/maintenanceproxy.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"

namespace client { namespace dialogs {

    /** Unpack dialog.
        Allows the user to configure an Unpack operation on the MaintenanceProxy,
        and executes it if so desired.

        @param proxy    MaintenanceProxy instance
        @param pHelp    Help widget (optional)
        @param root     UI Root
        @param tx       Translator

        @return set of unpacked players (file system was changed); empty if dialog was cancelled */
    game::PlayerSet_t doUnpackDialog(game::proxy::MaintenanceProxy& proxy, ui::Widget* pHelp, ui::Root& root, afl::string::Translator& tx);

} }

#endif
