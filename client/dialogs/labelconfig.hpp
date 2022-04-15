/**
  *  \file client/dialogs/labelconfig.hpp
  *  \brief Label configuration dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_LABELCONFIG_HPP
#define C2NG_CLIENT_DIALOGS_LABELCONFIG_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Label configuration dialog.
        Allows the user to edit the label expressions; see game::proxy::LabelProxy::setConfiguration().

        @param [in]     root       UI root
        @param [in]     tx         Translator
        @param [in]     gameSender Sender to access game data */
    void editLabelConfiguration(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender);

} }

#endif
