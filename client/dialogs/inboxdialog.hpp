/**
  *  \file client/dialogs/inboxdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_INBOXDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_INBOXDIALOG_HPP

#include "client/downlink.hpp"
#include "client/session.hpp"
#include "client/si/control.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "client/widgets/messageactionpanel.hpp"
#include "game/proxy/mailboxproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/rich/documentview.hpp"

namespace client { namespace dialogs {

    class InboxDialog : public client::si::Control {
     public:
        InboxDialog(String_t title, util::RequestSender<game::proxy::MailboxAdaptor> sender, client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx);
        ~InboxDialog();

        bool run(client::si::OutputState& out,
                 String_t helpPage,
                 String_t noMessageAdvice);

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link);
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text);
        virtual client::si::ContextProvider* createContextProvider();

     private:
        Downlink m_link;
        String_t m_title;

        void onUpdate(size_t index, const game::proxy::MailboxProxy::Message& msg);
        void updateButton(client::widgets::MessageActionPanel::Action a, const String_t& s);
        void onAction(client::widgets::MessageActionPanel::Action a, int arg);

        game::proxy::MailboxProxy::Status m_state;
        game::proxy::MailboxProxy::Message m_data;

        client::si::OutputState m_outputState;
        ui::EventLoop m_loop;

        client::widgets::MessageActionPanel m_actionPanel;
        ui::rich::DocumentView m_content;

        game::proxy::MailboxProxy m_proxy;
    };

} }

#endif
