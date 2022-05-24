/**
  *  \file client/dialogs/cloneship.hpp
  *  \brief Ship cloning dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_CLONESHIP_HPP
#define C2NG_CLIENT_DIALOGS_CLONESHIP_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Clone a ship.
        Displays a dialog to control a CloneShipProxy.
        @param root       UI root
        @param tx         Translator
        @param gameSender Game sender
        @param shipId     Id of a sufficient ship */
    void doCloneShip(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::Id_t shipId);

} }

#endif
