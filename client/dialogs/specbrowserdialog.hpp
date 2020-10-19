/**
  *  \file client/dialogs/specbrowserdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_SPECBROWSERDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SPECBROWSERDIALOG_HPP

#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "afl/string/translator.hpp"

namespace client { namespace dialogs {

    void doSpecificationBrowserDialog(ui::Root& root,
                                      util::RequestSender<game::Session> gameSender,
                                      afl::string::Translator& tx);

} }

#endif
