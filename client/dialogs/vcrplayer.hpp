/**
  *  \file client/dialogs/vcrplayer.hpp
  *  \brief VCR Player Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_VCRPLAYER_HPP
#define C2NG_CLIENT_DIALOGS_VCRPLAYER_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** VCR Player Dialog.
        Implements the common switch between VCR types.

        \param root       UI root
        \param tx         Translator
        \param vcrSender  Access to VCRs
        \param gameSender Access to game session
        \param log        Logger */
    game::Reference playCombat(ui::Root& root,
                               afl::string::Translator& tx,
                               util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender,
                               util::RequestSender<game::Session> gameSender,
                               afl::sys::LogListener& log);

} }

#endif
