/**
  *  \file client/dialogs/buildqueuesummary.hpp
  *  \brief Build Queue Summary Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDQUEUESUMMARY_HPP
#define C2NG_CLIENT_DIALOGS_BUILDQUEUESUMMARY_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/buildqueueproxy.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Build Queue Summary Dialog.

        Displays a summary of BuildQueueProxy::Infos_t (ChangeBuildQueue::Infos_t):
        - totals by action
        - totals by hull type

        \param infos       Information to display
        \param root        UI root
        \param gameSender  Game sender (for help, SelectionProxy)
        \param tx          Translator */
    void doBuildQueueSummaryDialog(const game::proxy::BuildQueueProxy::Infos_t& infos,
                                   ui::Root& root,
                                   util::RequestSender<game::Session> gameSender,
                                   afl::string::Translator& tx);

} }

#endif
