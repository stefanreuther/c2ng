/**
  *  \file client/dialogs/objectselectiondialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_OBJECTSELECTIONDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_OBJECTSELECTIONDIALOG_HPP

#include "client/si/userside.hpp"
#include "ui/root.hpp"
#include "afl/base/optional.hpp"
#include "client/si/requestlink2.hpp"
#include "client/si/outputstate.hpp"

namespace client { namespace dialogs {

    struct ObjectSelectionDialog {
        int screenNumber;
        const char* keymapName;
        const char* layoutName;
        const char* titleUT;
        const char* failMessageUT;
    };

    extern const ObjectSelectionDialog SHIP_SELECTION_DIALOG;
    extern const ObjectSelectionDialog PLANET_SELECTION_DIALOG;
    extern const ObjectSelectionDialog BASE_SELECTION_DIALOG;
    extern const ObjectSelectionDialog FLEET_SELECTION_DIALOG;

    int doObjectSelectionDialog(const ObjectSelectionDialog& def,
                                client::si::UserSide& iface,
                                client::si::Control& parentControl,
                                client::si::OutputState& outputState);

} }

#endif
