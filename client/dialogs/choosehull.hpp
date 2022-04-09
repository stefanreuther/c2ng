/**
  *  \file client/dialogs/choosehull.hpp
  *  \brief Hull selection dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_CHOOSEHULL_HPP
#define C2NG_CLIENT_DIALOGS_CHOOSEHULL_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Choose hull.
        Offers a list of hulls that can be filtered by player and sorted by name or Id.
        @param [in]     root       UI root
        @param [in]     title      Dialog title
        @param [in,out] hullNr     Current hull number
        @param [in]     tx         Translator
        @param [in]     gameSender Sender to access game data
        @param [in]     withCustom If true, include "Custom ship" entry with Id 0 */
    bool chooseHull(ui::Root& root, const String_t& title, int32_t& hullNr, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, bool withCustom);

} }

#endif
