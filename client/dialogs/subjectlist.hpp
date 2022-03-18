/**
  *  \file client/dialogs/subjectlist.hpp
  *  \brief Message Subject List dialog.
  */
#ifndef C2NG_CLIENT_DIALOGS_SUBJECTLIST_HPP
#define C2NG_CLIENT_DIALOGS_SUBJECTLIST_HPP

#include "afl/base/optional.hpp"
#include "afl/string/translator.hpp"
#include "game/msg/browser.hpp"
#include "game/proxy/mailboxproxy.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Message subject list dialog.
        Displays the list of subjects (game::msg::Browser::Summary_t).
        Allows the user to perform operations on proxy:
        - toggle filterd status, toggleHeadingFiltered()
        - select a message for display, setCurrentMessage()
        \param [in,out] proxy       Proxy to work on
        \param [in]     root        UI root
        \param [in]     gameSender  Game sender for related operations
        \param [in]     tx          Translator */
    void doSubjectListDialog(game::proxy::MailboxProxy& proxy,
                             ui::Root& root,
                             util::RequestSender<game::Session> gameSender,
                             afl::string::Translator& tx);

} }

#endif
