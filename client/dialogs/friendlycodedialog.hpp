/**
  *  \file client/dialogs/friendlycodedialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_FRIENDLYCODEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_FRIENDLYCODEDIALOG_HPP

#include "afl/string/translator.hpp"
#include "client/widgets/friendlycodelist.hpp"
#include "game/session.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "ui/root.hpp"
#include "ui/widgets/inputline.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    class FriendlyCodeDialog {
     public:
        FriendlyCodeDialog(ui::Root& root, afl::string::Translator& tx, const String_t& title, const game::spec::FriendlyCodeList::Infos_t& list, util::RequestSender<game::Session> gameSender);
        ~FriendlyCodeDialog();

        void setFriendlyCode(const String_t& code);
        String_t getFriendlyCode() const;

        bool run();

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        String_t m_title;
        util::RequestSender<game::Session> m_gameSender;
        ui::widgets::InputLine m_input;
        client::widgets::FriendlyCodeList m_list;

        void onListChange();
        void onInputChange();
        void onRandom();
    };

} }

#endif
