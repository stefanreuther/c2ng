/**
  *  \file client/cargotransfer.hpp
  */
#ifndef C2NG_CLIENT_CARGOTRANSFER_HPP
#define C2NG_CLIENT_CARGOTRANSFER_HPP

#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "afl/string/translator.hpp"
#include "game/actions/cargotransfersetup.hpp"

namespace client {

    void doCargoTransfer(ui::Root& root,
                         util::RequestSender<game::Session> gameSender,
                         afl::string::Translator& tx,
                         game::actions::CargoTransferSetup setup);

    void doShipCargoTransfer(ui::Root& root,
                             util::RequestSender<game::Session> gameSender,
                             afl::string::Translator& tx,
                             game::Id_t shipId);

    void doPlanetCargoTransfer(ui::Root& root,
                               util::RequestSender<game::Session> gameSender,
                               afl::string::Translator& tx,
                               game::Id_t planetId,
                               bool unload);

}

#endif
