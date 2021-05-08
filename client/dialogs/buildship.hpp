/**
  *  \file client/dialogs/buildship.hpp
  *  \brief Ship Building Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDSHIP_HPP
#define C2NG_CLIENT_DIALOGS_BUILDSHIP_HPP

#include "afl/base/types.hpp"
#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Ship Building Dialog.
        Controls a game::proxy::BuildShipProxy.
        \param root       UI root
        \param gameSender Game sender
        \param planetId   Planet Id
        \param tx         Translator */
    void doBuildShip(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, afl::string::Translator& tx);

} }

#endif
