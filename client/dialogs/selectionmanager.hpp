/**
  *  \file client/dialogs/selectionmanager.hpp
  *  \brief Selection Manager dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_SELECTIONMANAGER_HPP
#define C2NG_CLIENT_DIALOGS_SELECTIONMANAGER_HPP

#include "afl/base/optional.hpp"
#include "client/si/control.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "game/searchquery.hpp"

namespace client { namespace dialogs {

    /** Selection manager, main entry point.
        Implements the selection manager and possible search dialog invoked from it.
        If may produce an outbound process, e.g. when a control screen is activated.

        @param [in]  iface   UserSide
        @param [out] out     Output State (outbound process) */
    void doSelectionManager(client::si::UserSide& iface, client::si::OutputState& out);

    /** Selection manager, entry point from search dialog.
        Implements the selection manager.
        If user requests the search dialog, returns the desired search query instead;
        caller must operate the search dialog.

        @param [in]  iface   UserSide
        @param [out] out     Output State (outbound process)

        @return Search query to activate search dialog with */
    afl::base::Optional<game::SearchQuery> doSelectionManagerFromSearch(client::si::UserSide& iface,
                                                                        client::si::OutputState& out);

} }

#endif
