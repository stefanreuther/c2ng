/**
  *  \file client/dialogs/attachmentselection.hpp
  *  \brief Attachment Selection Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_ATTACHMENTSELECTION_HPP
#define C2NG_CLIENT_DIALOGS_ATTACHMENTSELECTION_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/attachmentproxy.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Choose attachments.
        @param [in,out] infos        Attachment information. Dialog will update the @c selected members.
        @param [in]     gameSession  Game session (for help dialog)
        @param [in]     root         UI root
        @param [in]     tx           Translator */
    bool chooseAttachments(game::proxy::AttachmentProxy::Infos_t& infos, util::RequestSender<game::Session> gameSender, ui::Root& root, afl::string::Translator& tx);

} }

#endif
