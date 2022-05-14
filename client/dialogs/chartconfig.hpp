/**
  *  \file client/dialogs/chartconfig.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_CHARTCONFIG_HPP
#define C2NG_CLIENT_DIALOGS_CHARTCONFIG_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    void doChartConfigDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

} }

#endif
