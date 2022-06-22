/**
  *  \file client/dialogs/searchdialog.hpp
  *  \brief Search Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_SEARCHDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SEARCHDIALOG_HPP

#include "afl/string/translator.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "game/searchquery.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** "Search" dialog.
        Displays and operates the regular standalone search dialog.

        @param [in]  initialQuery     Initial search query
        @param [in]  currentObject    Current object; if search result contains this object, focus it.
        @param [in]  immediate        If true, immediately perform a search
        @param [in]  iface            Connection to game side
        @param [out] out              Output state, e.g. order to change to a control screen */
    void doSearchDialog(const game::SearchQuery& initialQuery,
                        game::Reference currentObject,
                        bool immediate,
                        client::si::UserSide& iface,
                        client::si::OutputState& out);

} }

#endif
