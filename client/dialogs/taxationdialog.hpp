/**
  *  \file client/dialogs/taxationdialog.hpp
  *  \brief Taxation Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_TAXATIONDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_TAXATIONDIALOG_HPP

#include "afl/base/optional.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Taxation dialog.
        \param planetId      Planet Id to work on
        \param numBuildings  Number of buildings (mines + factories) override
        \param root          UI root
        \param tx            Translator
        \param gameSender    Game sender */
    void doTaxationDialog(game::Id_t planetId,
                          afl::base::Optional<int> numBuildings,
                          ui::Root& root,
                          afl::string::Translator& tx,
                          util::RequestSender<game::Session> gameSender);

} }

#endif
