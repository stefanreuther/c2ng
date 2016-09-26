/**
  *  \file client/dialogs/consoledialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_CONSOLEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_CONSOLEDIALOG_HPP

#include "client/si/userside.hpp"
#include "ui/root.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/inputstate.hpp"

namespace client { namespace dialogs {

    bool doConsoleDialog(client::si::UserSide& iface,
                         client::si::Control& parentControl,
                         client::si::InputState& inputState,
                         client::si::OutputState& outputState);

} }

#endif
