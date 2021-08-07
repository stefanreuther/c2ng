/**
  *  \file client/dialogs/hullspecification.hpp
  *  \brief Hull Specification Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_HULLSPECIFICATION_HPP
#define C2NG_CLIENT_DIALOGS_HULLSPECIFICATION_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/shipquery.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Show hull specification dialog for a ship.
        Displays the dialog and offers sub-dialogs, but no other interaction.
        \param shipId     Ship Id
        \param root       UI root
        \param tx         Translator
        \param gameSender Game sender (for PlayerProxy, ConfigurationProxy, HullSpecificationProxy, help) */
    void showHullSpecificationForShip(game::Id_t shipId, ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender);

    /** Show hull specification dialog for a ship query (i.e. hypothetical ship).
        Displays the dialog and offers sub-dialogs, but no other interaction.
        \param q          ShipQuery object
        \param root       UI root
        \param tx         Translator
        \param gameSender Game sender (for PlayerProxy, ConfigurationProxy, HullSpecificationProxy, help) */
    void showHullSpecification(const game::ShipQuery& q, ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender);

} }

#endif
