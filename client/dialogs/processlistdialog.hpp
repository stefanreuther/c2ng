/**
  *  \file client/dialogs/processlistdialog.hpp
  *  \brief Process List Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_PROCESSLISTDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_PROCESSLISTDIALOG_HPP

#include "client/si/control.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "game/reference.hpp"

namespace client { namespace dialogs {

    void doProcessListDialog(game::Reference invokingObject,
                             client::si::UserSide& iface,
                             client::si::Control& ctl,
                             client::si::OutputState& out);
    
} }

#endif
