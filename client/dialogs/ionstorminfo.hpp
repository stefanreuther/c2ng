/**
  *  \file client/dialogs/ionstorminfo.hpp
  *  \brief Ion Storm Information Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_IONSTORMINFO_HPP
#define C2NG_CLIENT_DIALOGS_IONSTORMINFO_HPP

#include "afl/string/translator.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Show ion storm information dialog.
        Uses an IonStormProxy to look at an ion storm.
        This means the storm needs to be selected on the ion storm cursor.
        \param [in]  iface  Script interface
        \param [in]  root   UI root
        \param [in]  tx     Translator
        \param [out] out    Output state, can contain a new screen to go to */
    void doIonStormInfoDialog(client::si::UserSide& iface,
                              ui::Root& root,
                              afl::string::Translator& tx,
                              client::si::OutputState& out);

} }

#endif
