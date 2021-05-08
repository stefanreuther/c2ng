/**
  *  \file client/dialogs/multitransfer.hpp
  *  \brief Multi-Ship Cargo Transfer
  */
#ifndef C2NG_CLIENT_DIALOGS_MULTITRANSFER_HPP
#define C2NG_CLIENT_DIALOGS_MULTITRANSFER_HPP

#include "afl/string/translator.hpp"
#include "game/actions/multitransfersetup.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Multi-Ship Cargo Transfer

        \param setup       Setup (ship, fleet flag, type)
        \param gameSender  Game sender (for proxies)
        \param elementName Name of element to transfer (for UI headings)
        \param root        UI root
        \param tx          Translator */
    void doMultiTransfer(game::actions::MultiTransferSetup setup,
                         util::RequestSender<game::Session> gameSender,
                         String_t elementName,
                         ui::Root& root,
                         afl::string::Translator& tx);

} }

#endif
