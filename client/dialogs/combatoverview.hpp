/**
  *  \file client/dialogs/combatoverview.hpp
  *  \brief Combat Overview dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_COMBATOVERVIEW_HPP
#define C2NG_CLIENT_DIALOGS_COMBATOVERVIEW_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Show Combat Overview dialog.
        This dialog shows a summary of a VCR database and allows choosing a battle.

        \param [in]  root         UI root
        \param [in]  tx           Translator
        \param [in]  vcrSender    Access to desired VCR database
        \param [in]  gameSender   Access to game session (for names, config, etc.)
        \param [out] chosenBattle Chosen battle number
        \retval true User chose a battle; chosenBattle has been set
        \retval false User canceled dialog normally */
    bool showCombatOverview(ui::Root& root,
                            afl::string::Translator& tx,
                            util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender,
                            util::RequestSender<game::Session> gameSender,
                            size_t& chosenBattle);

} }

#endif
