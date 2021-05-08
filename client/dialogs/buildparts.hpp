/**
  *  \file client/dialogs/buildparts.hpp
  *  \brief Starship Part Building
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDPARTS_HPP
#define C2NG_CLIENT_DIALOGS_BUILDPARTS_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Dialog for building starship parts.
        Controls a game::proxy::BuildShipProxy.
        \param root       UI root
        \param gameSender Game sender
        \param planetId   Planet Id
        \param area       Area
        \param partId     Part Id (hull Id, engine Id, ...)
        \param tx         Translator */
    void doBuildShipParts(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, game::TechLevel area, int partId, afl::string::Translator& tx);

} }

#endif
