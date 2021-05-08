/**
  *  \file client/dialogs/notifications.hpp
  *  \brief Notifications
  */
#ifndef C2NG_CLIENT_DIALOGS_NOTIFICATIONS_HPP
#define C2NG_CLIENT_DIALOGS_NOTIFICATIONS_HPP

#include "afl/base/optional.hpp"
#include "afl/string/translator.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "game/proxy/processlistproxy.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Show notifications.

        Displays notifications and lets user deal with them.
        When leaving the dialog, uses the given ProcessListProxy to mark processes to run again
        when their notifications were confirmed.
        
        \param [in] processId   Try to show this process' notification
        \param [in] plProxy     ProcessListProxy
        \param [in] iface       UserSide
        \param [in] root        UI root
        \param [in] tx          Translator
        \param [out] out        Output state */
    void showNotifications(afl::base::Optional<uint32_t> processId,
                           game::proxy::ProcessListProxy& plProxy,
                           client::si::UserSide& iface,
                           ui::Root& root,
                           afl::string::Translator& tx,
                           client::si::OutputState& out);

} }

#endif
