/**
  *  \file client/dialogs/combatscoresummary.hpp
  *  \brief Combat Score Summary dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_COMBATSCORESUMMARY_HPP
#define C2NG_CLIENT_DIALOGS_COMBATSCORESUMMARY_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Show Combat Score Summary dialog.
        This dialog shows a score summary of a VCR database.

        \param root         UI root
        \param tx           Translator
        \param vcrSender    Access to desired VCR database
        \param gameSender   Access to game session (for names, config, etc.) */
    void showCombatScoreSummary(ui::Root& root,
                                afl::string::Translator& tx,
                                util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender,
                                util::RequestSender<game::Session> gameSender);


} }

#endif
