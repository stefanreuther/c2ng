/**
  *  \file client/dialogs/searchdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_SEARCHDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SEARCHDIALOG_HPP

#include "afl/string/translator.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "game/searchquery.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    void doSearchDialog(const game::SearchQuery& initialQuery,
                        game::Reference currentObject,
                        bool immediate,
                        client::si::UserSide& iface,
                        ui::Root& root,
                        afl::string::Translator& tx,
                        client::si::OutputState& out);

} }

#endif
