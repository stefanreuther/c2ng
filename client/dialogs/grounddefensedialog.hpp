/**
  *  \file client/dialogs/grounddefensedialog.hpp
  *  \brief Ground Defense Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_GROUNDDEFENSEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_GROUNDDEFENSEDIALOG_HPP

#include "ui/root.hpp"
#include "game/map/planetinfo.hpp"
#include "afl/string/translator.hpp"

namespace client { namespace dialogs {

    /** Show ground defense information.
        \param root Root
        \param info Information to display
        \param tx   Translator */
    void doGroundDefenseDialog(ui::Root& root, const game::map::GroundDefenseInfo& info, afl::string::Translator& tx);

} }

#endif
