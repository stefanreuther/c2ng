/**
  *  \file client/dialogs/inboxdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_INBOXDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_INBOXDIALOG_HPP

#include "client/downlink.hpp"
#include "client/session.hpp"
#include "client/si/control.hpp"
#include "client/si/outputstate.hpp"
#include "client/widgets/messageactionpanel.hpp"
#include "game/playerset.hpp"
#include "game/reference.hpp"
#include "ui/eventloop.hpp"
#include "ui/rich/documentview.hpp"
#include "client/si/userside.hpp"

namespace client { namespace dialogs {

    class InboxDialog : public client::si::Control {
     public:
        InboxDialog(client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx);
        ~InboxDialog();

        void run(client::si::OutputState& out);

        virtual void handleStateChange(client::si::UserSide& us, client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::UserSide& us, client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::UserSide& us, client::si::RequestLink2 link);
        virtual void handleSetViewRequest(client::si::UserSide& ui, client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual client::si::ContextProvider* createContextProvider();

     private:
        ui::Root& m_root;
        Downlink m_link;

        class Query;
        class InitQuery;
        class BrowseQuery;
        class FirstQuery;
        class LastQuery;
        class LoadQuery;

        struct State {
            size_t current;     // 0-based
            size_t limit;
            bool dim;
            game::Reference goto1;
            String_t goto1Name;
            game::Reference goto2;
            String_t goto2Name;
            game::PlayerSet_t reply;
            game::PlayerSet_t replyAll;
            String_t replyName;
            util::rich::Text text;

            State()
                : current(0), limit(0), dim(false),
                  goto1(), goto1Name(), goto2(), goto2Name(), reply(), replyAll(), replyName(),
                  text()
                { }
            void load(game::Session& session, size_t index);
        };

        void setState(const State& state);
        void updateButton(client::widgets::MessageActionPanel::Action a, const String_t& s);
        void onAction(client::widgets::MessageActionPanel::Action a, int arg);

        State m_state;

        client::si::OutputState m_outputState;
        ui::EventLoop m_loop;

        client::widgets::MessageActionPanel m_actionPanel;
        ui::rich::DocumentView m_content;
    };

} }

#endif
