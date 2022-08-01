/**
  *  \file client/dialogs/changepassword.hpp
  *  \brief Password change dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_CHANGEPASSWORD_HPP
#define C2NG_CLIENT_DIALOGS_CHANGEPASSWORD_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Change password.
        @param [in]  root       UI root
        @param [in]  tx         Translator
        @param [out] result     Result */
    bool doChangePassword(ui::Root& root, afl::string::Translator& tx, String_t& result);

} }

#endif
