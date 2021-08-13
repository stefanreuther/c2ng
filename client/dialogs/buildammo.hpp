/**
  *  \file client/dialogs/buildammo.hpp
  *  \brief Ammo Building Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDAMMO_HPP
#define C2NG_CLIENT_DIALOGS_BUILDAMMO_HPP

#include "afl/base/types.hpp"
#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/proxy/buildammoproxy.hpp"

namespace client { namespace dialogs {

    /** Ammo Building Dialog.
        Controls a game::proxy::BuildAmmoProxy.
        \param root       UI root
        \param proxy      BuildAmmoProxy, configured for initial target (ship or planet)
        \param gameSender Game sender
        \param planetId   Planet Id
        \param tx         Translator */
    void doBuildAmmo(ui::Root& root,
                     game::proxy::BuildAmmoProxy& proxy,
                     util::RequestSender<game::Session> gameSender,
                     game::Id_t planetId,
                     afl::string::Translator& tx);

} }

#endif
