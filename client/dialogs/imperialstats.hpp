/**
  *  \file client/dialogs/imperialstats.hpp
  *  \brief Imperial Statistics dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_IMPERIALSTATS_HPP
#define C2NG_CLIENT_DIALOGS_IMPERIALSTATS_HPP

#include "client/si/userside.hpp"
#include "client/si/outputstate.hpp"

namespace client { namespace dialogs {

    /** Show "Imperial Statistics" dialog.
        This dialog displays information and offers links that start script commands
        (UI.GotoScreen, UI.Search).

        @param userSide     UserSide
        @param outputState  OutputState produced by script */
    void doImperialStatistics(client::si::UserSide& userSide, client::si::OutputState& outputState);

} }

#endif
