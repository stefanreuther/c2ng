/**
  *  \file client/dialogs/outboxdialog.hpp
  *  \brief Class client::dialogs::OutboxDialog
  */
#ifndef C2NG_CLIENT_DIALOGS_OUTBOXDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_OUTBOXDIALOG_HPP

#include "client/downlink.hpp"
#include "client/si/control.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "client/widgets/messageactionpanel.hpp"
#include "game/proxy/mailboxproxy.hpp"
#include "game/proxy/outboxproxy.hpp"
#include "game/session.hpp"
#include "ui/eventloop.hpp"
#include "ui/rich/documentview.hpp"

namespace client { namespace dialogs {

    class OutboxDialog : public client::si::Control {
     public:
        OutboxDialog(String_t title, client::si::UserSide& iface, ui::Root& root, String_t helpPage, afl::string::Translator& tx);
        ~OutboxDialog();

        bool run(client::si::OutputState& out,
                 String_t noMessageAdvice);

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link);
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text);
        virtual game::interface::ContextProvider* createContextProvider();

     private:
        Downlink m_link;
        String_t m_title;

        void onUpdate(size_t index, const game::proxy::MailboxProxy::Message& msg);
        void updateButton(client::widgets::MessageActionPanel::Action a, const String_t& s);
        void onAction(client::widgets::MessageActionPanel::Action a, int arg);
        void editMessage();
        void redirectMessage();
        void deleteMessage();
        void reload();

        game::proxy::MailboxProxy::Status m_state;
        game::proxy::MailboxProxy::Message m_data;

        client::si::OutputState m_outputState;
        ui::EventLoop m_loop;

        client::widgets::MessageActionPanel m_actionPanel;
        ui::rich::DocumentView m_content;

        String_t m_helpPage;

        game::proxy::OutboxProxy m_outboxProxy;
        game::proxy::MailboxProxy m_proxy;
    };

} }


#endif
