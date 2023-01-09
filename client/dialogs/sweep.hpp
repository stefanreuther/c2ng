/**
  *  \file client/dialogs/sweep.hpp
  *  \brief Directory Cleanup (Sweep) Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_SWEEP_HPP
#define C2NG_CLIENT_DIALOGS_SWEEP_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/maintenanceproxy.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"

namespace client { namespace dialogs {

    /** Do Directory Cleanup (Sweep) Dialog.
        Allows the user to configure a Sweep operation on the MaintenanceProxy,
        and executes it if so desired.

        @param proxy    MaintenanceProxy instance
        @param pHelp    Help widget (optional)
        @param root     UI Root
        @param tx       Translator

        @return true if user executed the operation (file system was changed) */
    bool doSweepDialog(game::proxy::MaintenanceProxy& proxy, ui::Widget* pHelp, ui::Root& root, afl::string::Translator& tx);

} }

#endif
