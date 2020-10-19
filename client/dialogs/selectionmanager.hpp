/**
  *  \file client/dialogs/selectionmanager.hpp
  *  \brief Class client::dialogs::SelectionManager
  */
#ifndef C2NG_CLIENT_DIALOGS_SELECTIONMANAGER_HPP
#define C2NG_CLIENT_DIALOGS_SELECTIONMANAGER_HPP

#include "client/si/userside.hpp"
#include "client/si/control.hpp"
#include "client/si/outputstate.hpp"

namespace client { namespace dialogs {

    void doSelectionManager(client::si::UserSide& iface,
                            client::si::Control& ctl,
                            client::si::OutputState& out);

} }

#endif
