/**
  *  \file client/dialogs/specbrowserdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_SPECBROWSERDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SPECBROWSERDIALOG_HPP

#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "afl/string/translator.hpp"
#include "ui/rich/document.hpp"
#include "game/spec/info/types.hpp"

namespace client { namespace dialogs {

    void doSpecificationBrowserDialog(ui::Root& root,
                                      util::RequestSender<game::Session> gameSender,
                                      afl::string::Translator& tx);

    void renderHullInformation(ui::rich::Document& doc, ui::Root& root, const game::spec::info::PageContent& content);

} }

#endif
