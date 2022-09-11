/**
  *  \file client/dialogs/globalactions.hpp
  *  \brief Global Actions dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_GLOBALACTIONS_HPP
#define C2NG_CLIENT_DIALOGS_GLOBALACTIONS_HPP

#include <memory>
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "game/ref/list.hpp"
#include "interpreter/variablereference.hpp"

namespace client { namespace dialogs {

    /** Global Actions dialog.
        Displays the list of actions and lets the user choose, configure and execute them.

        The actions must be provided in a script variable, which is referenced using a VariableReference.

        @param [in,out] us            Connection to game side
        @param [out]    outputState   Possible status change
        @param [in,out] searchResult  Search result. Allows passing in a search result,
                                      and otherwise operates as workspace for acquiring a new result.
        @param [in]     ref           Reference to variable containing the global actions */
    void doGlobalActions(client::si::UserSide& us,
                         client::si::OutputState& outputState,
                         game::ref::List& searchResult,
                         interpreter::VariableReference ref);

} }

#endif
