/**
  *  \file client/dialogs/techupgradedialog.hpp
  *  \brief Tech Upgrade Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_TECHUPGRADEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_TECHUPGRADEDIALOG_HPP

#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/spec/cost.hpp"

namespace client { namespace dialogs {

    /** Tech upgrade dialog.

        \param root        UI root
        \param tx          Translator
        \param gameSender  Game sender
        \param pid         Planet Id */
    void doTechUpgradeDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::Id_t pid);

    /** One-shot tech upgrade dialog.

        Used as part of other UI flows.

        \param root        UI root
        \param tx          Translator
        \param gameSender  Game sender
        \param pid         Planet Id
        \param ind         WaitIndicator. Passed in to allow re-use of the outer UI flow's WaitIndicator.
        \param area        Area to upgrade
        \param level       Level to upgrade to
        \param reservedAmount Reserved cargo amount
        \param introFormat Introductory sentence for upgrade, must include a "%d" placeholder ("You need tech %d to buy this.")
        \param title       Window title

        \retval true Tech level is available (was available before, or got bought)
        \retval false Tech level is not available (cannot be bought, user canceled) */
    bool checkTechUpgrade(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::Id_t pid,
                          game::proxy::WaitIndicator& ind, game::TechLevel area, int level, game::spec::Cost reservedAmount,
                          String_t introFormat, String_t title);

} }

#endif
