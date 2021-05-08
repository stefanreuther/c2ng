/**
  *  \file client/dialogs/ufoinfo.hpp
  *  \brief Ufo information dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_UFOINFO_HPP
#define C2NG_CLIENT_DIALOGS_UFOINFO_HPP

#include "afl/string/translator.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Show ufo information dialog.
        Uses an UfoProxy to look at an Ufo.
        This means the Ufo needs to be selected on the Ufo cursor.
        \param [in]  iface  Script interface
        \param [in]  root   UI root
        \param [in]  tx     Translator
        \param [out] out    Output state, can contain a new screen to go to */
    void doUfoInfoDialog(client::si::UserSide& iface,
                         ui::Root& root,
                         afl::string::Translator& tx,
                         client::si::OutputState& out);

} }

#endif
