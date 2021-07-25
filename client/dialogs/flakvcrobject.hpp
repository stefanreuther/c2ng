/**
  *  \file client/dialogs/flakvcrobject.hpp
  *  \brief FLAK VCR Object Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_FLAKVCROBJECT_HPP
#define C2NG_CLIENT_DIALOGS_FLAKVCROBJECT_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/vcrdatabaseproxy.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Show FLAK VCR Object Information Dialog.
        Shows a list and details of all participants.
        \param root        Root
        \param tx          Translator
        \param gameSender  Game sender (for ConfigurationProxy)
        \param proxy       VcrDatabaseProxy to use
        \param info        Battle info
        \return If user chose to go to an object's control screen, a reference to it. */
    game::Reference doFlakVcrObjectInfoDialog(ui::Root& root,
                                              afl::string::Translator& tx,
                                              util::RequestSender<game::Session> gameSender,
                                              game::proxy::VcrDatabaseProxy& proxy,
                                              const game::vcr::BattleInfo& info);

} }

#endif
