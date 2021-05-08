/**
  *  \file client/dialogs/commandlistdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_COMMANDLISTDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_COMMANDLISTDIALOG_HPP

#include "client/si/userside.hpp"
#include "client/si/control.hpp"
#include "client/si/outputstate.hpp"

namespace client { namespace dialogs {

    void editCommands(ui::Root& root,
                      client::si::UserSide& iface,
                      client::si::OutputState& outputState,
                      afl::string::Translator& tx);

} }

#endif
