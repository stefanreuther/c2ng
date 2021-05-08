/**
  *  \file client/dialogs/minefieldinfo.hpp
  *  \brief Minefield Information Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_MINEFIELDINFO_HPP
#define C2NG_CLIENT_DIALOGS_MINEFIELDINFO_HPP

#include "afl/string/translator.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Show minefield information dialog.
        Uses a MinefieldProxy to look at a minefield.
        This means the minefield needs to be selected on the Minefield cursor.
        \param [in]  iface  Script interface
        \param [in]  root   UI root
        \param [in]  tx     Translator
        \param [out] out    Output state, can contain a new screen to go to */
    void doMinefieldInfoDialog(client::si::UserSide& iface,
                               ui::Root& root,
                               afl::string::Translator& tx,
                               client::si::OutputState& out);

} }

#endif
