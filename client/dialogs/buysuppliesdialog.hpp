/**
  *  \file client/dialogs/buysuppliesdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_BUYSUPPLIESDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_BUYSUPPLIESDIALOG_HPP

#include "afl/base/types.hpp"
#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    void doBuySuppliesDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, int32_t reservedMoney, int32_t reservedSupplies, afl::string::Translator& tx);

} }

#endif
