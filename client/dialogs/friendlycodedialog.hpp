/**
  *  \file client/dialogs/friendlycodedialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_FRIENDLYCODEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_FRIENDLYCODEDIALOG_HPP

#include "client/widgets/friendlycodelist.hpp"
#include "game/data/friendlycode.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "ui/widgets/inputline.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    class FriendlyCodeDialog {
     public:
        FriendlyCodeDialog(ui::Root& root, const String_t& title, const game::data::FriendlyCodeList_t& list, util::RequestSender<game::Session> gameSender);
        ~FriendlyCodeDialog();

        void setFriendlyCode(const String_t& code);
        String_t getFriendlyCode() const;

        bool run();

     private:
        ui::Root& m_root;
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
