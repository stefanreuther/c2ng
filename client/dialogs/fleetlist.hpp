/**
  *  \file client/dialogs/fleetlist.hpp
  *  \brief Fleet list standard dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_FLEETLIST_HPP
#define C2NG_CLIENT_DIALOGS_FLEETLIST_HPP

#include "ui/root.hpp"
#include "game/ref/fleetlist.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "afl/string/translator.hpp"

namespace client { namespace dialogs {

    /** Fleet list standard dialog.
        @param root       UI root
        @param okLabel    Name of "ok" button
        @param title      Dialog title
        @param list       List of fleets to display (contains references, dividers, etc.)
        @param gameSender Game sender (for help)
        @param tx         Translator
        @return chosen reference; empty reference if dialog cancelled */
    game::Reference doFleetList(ui::Root& root,
                                String_t okLabel,
                                String_t title,
                                const game::ref::FleetList& list,
                                util::RequestSender<game::Session> gameSender,
                                afl::string::Translator& tx);

} }

#endif
