/**
  *  \file client/dialogs/preferences.hpp
  *  \brief Preferences Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_PREFERENCES_HPP
#define C2NG_CLIENT_DIALOGS_PREFERENCES_HPP

#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "interpreter/variablereference.hpp"

namespace client { namespace dialogs {

    /** Preferences dialog.

        The preference editor is script-controlled/extensible.
        Therefore, this function takes references to script variables (with option definitions),
        and can potentially produce an outbound process.

        @param [in]  us           UserSide (for game sender, UI root, translator, control)
        @param [in]  options      Prepared list of game options
        @param [out] out          OutputState */
    void doPreferencesDialog(client::si::UserSide& us,
                             const interpreter::VariableReference& options,
                             client::si::OutputState& out);

} }

#endif
