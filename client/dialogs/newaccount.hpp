/**
  *  \file client/dialogs/newaccount.hpp
  *  \brief Account Creation dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_NEWACCOUNT_HPP
#define C2NG_CLIENT_DIALOGS_NEWACCOUNT_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/browserproxy.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"

namespace client { namespace dialogs {

    /** Account Creation dialog.
        Asks user for account parameters, and creates the account using the BrowserProxy.
        @param proxy BrowserProxy instance
        @param pHelp Help widget (optional)
        @param root  UI root
        @param tx    Translator
        @return true if account was created, false if dialog was canceled */
    bool doNewAccountDialog(game::proxy::BrowserProxy& proxy, ui::Widget* pHelp, ui::Root& root, afl::string::Translator& tx);

} }

#endif
