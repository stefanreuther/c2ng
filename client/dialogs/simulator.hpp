/**
  *  \file client/dialogs/simulator.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATOR_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATOR_HPP

#include "client/si/userside.hpp"
#include "client/si/control.hpp"
#include "client/si/outputstate.hpp"

namespace client { namespace dialogs {

    void doBattleSimulator(client::si::UserSide& iface,
                           client::si::Control& ctl,
                           client::si::OutputState& outputState);

} }

#endif
