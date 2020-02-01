/**
  *  \file client/dialogs/sellsuppliesdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_SELLSUPPLIESDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_SELLSUPPLIESDIALOG_HPP

#include "afl/base/types.hpp"
#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    void doSellSuppliesDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, int32_t reservedSupplies, int32_t reservedMoney, afl::string::Translator& tx);

} }

#endif
