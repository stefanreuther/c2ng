/**
  *  \file client/dialogs/inboxdialog.cpp
  */

#include "client/dialogs/inboxdialog.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/subjectlist.hpp"
#include "client/widgets/decayingmessage.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/actions/preconditions.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"

using client::widgets::MessageActionPanel;

/****************************** InboxDialog ******************************/

client::dialogs::InboxDialog::InboxDialog(String_t title, util::RequestSender<game::proxy::MailboxAdaptor> sender, client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx)
    : Control(iface, root, tx),
      m_link(root, tx),
      m_title(title),
      m_state(),
      m_data(),
      m_outputState(),
      m_loop(root),
      m_actionPanel(root, tx),
      m_content(root.provider().getFont(gfx::FontRequest().setStyle(1))->getCellSize().scaledBy(41, 22), 0, root.provider()),
      m_proxy(sender, root.engine().dispatcher())
{
    m_proxy.sig_update.add(this, &InboxDialog::onUpdate);
}

client::dialogs::InboxDialog::~InboxDialog()
{ }

bool
client::dialogs::InboxDialog::run(client::si::OutputState& out,
                                  String_t helpPage,
                                  String_t noMessageAdvice)
{
    // Initialize messenger
    m_proxy.getStatus(m_link, m_state);
    if (m_state.numMessages == 0) {
        client::widgets::showDecayingMessage(root(), noMessageAdvice);
        return false;
    }

    // Window
    //   HBox
    //     VBox
    //       Actions
    //       HBox
    //         Close
    //         Spacer
    //   Content

    ui::Window win(m_title, root().provider(), root().colorScheme(), ui::BLUE_BLACK_WINDOW, ui::layout::HBox::instance5);
    ui::Group g1(ui::layout::VBox::instance5);
    g1.add(m_actionPanel);

    client::widgets::HelpWidget help(root(), translator(), interface().gameSender(), helpPage);

    ui::Group g12(ui::layout::HBox::instance5);
    ui::widgets::Button btnOK(translator()("OK"), util::Key_Escape, root());
    ui::widgets::Button btnHelp(translator()("Help"), 'h', root());
    ui::Spacer spc;
    ui::PrefixArgument prefix(root());
    ui::widgets::Quit quit(root(), m_loop);
    g12.add(btnOK);
    g12.add(spc);
    g12.add(btnHelp);
    g1.add(g12);
    win.add(g1);
    win.add(m_content);
    win.add(prefix);
    win.add(help);
    win.add(quit);

    btnOK.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnHelp.dispatchKeyTo(help);
    m_actionPanel.sig_action.add(this, &InboxDialog::onAction);

    win.pack();

    // Request current data
    m_proxy.setCurrentMessage(m_state.currentMessage);

    root().centerWidget(win);
    root().add(win);

    // Run (this will immediately exit if one of the above scripts requested a context change.)
    bool stateChanged = (m_loop.run() != 0);

    out = m_outputState;
    return stateChanged;
}

/*
 *  Control methods
 */

void
client::dialogs::InboxDialog::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    dialogHandleStateChange(link, target, m_outputState, m_loop, 1);
}

void
client::dialogs::InboxDialog::handleEndDialog(client::si::RequestLink2 link, int code)
{
    dialogHandleEndDialog(link, code, m_outputState, m_loop, 1);
}

void
client::dialogs::InboxDialog::handlePopupConsole(client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
client::dialogs::InboxDialog::handleSetViewRequest(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetViewRequest(link, name, withKeymap);
}

void
client::dialogs::InboxDialog::handleUseKeymapRequest(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymapRequest(link, name, prefix);
}

void
client::dialogs::InboxDialog::handleOverlayMessageRequest(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessageRequest(link, text);
}

client::si::ContextProvider*
client::dialogs::InboxDialog::createContextProvider()
{
    return 0;
}


/*
 *  InboxDialog Methods
 */

void
client::dialogs::InboxDialog::onUpdate(size_t index, const game::proxy::MailboxProxy::Message& msg)
{
    // ex WMessageActionPanel::onBrowse (sort-of)
    m_data = msg;

    // Position
    m_actionPanel.setPosition(afl::string::Format("%d/%d", index+1, m_state.numMessages), msg.isFiltered);

    // Buttons
    updateButton(MessageActionPanel::GoTo1, msg.goto1Name);
    updateButton(MessageActionPanel::GoTo2, msg.goto2Name);
    updateButton(MessageActionPanel::Reply, msg.replyName);

    if (msg.actions.contains(game::msg::Mailbox::ToggleConfirmed)) {
        m_actionPanel.enableAction(MessageActionPanel::Confirm, String_t());
    } else {
        m_actionPanel.disableAction(MessageActionPanel::Confirm);
    }

    // Content
    ui::rich::Document& doc = m_content.getDocument();
    doc.clear();
    doc.add(msg.text);
    doc.finish();
    m_content.handleDocumentUpdate();
}

void
client::dialogs::InboxDialog::updateButton(client::widgets::MessageActionPanel::Action a, const String_t& s)
{
    if (s.empty()) {
        m_actionPanel.disableAction(a);
    } else {
        m_actionPanel.enableAction(a, s);
    }
}

void
client::dialogs::InboxDialog::onAction(client::widgets::MessageActionPanel::Action a, int arg)
{
    switch (a) {
     case MessageActionPanel::GoTo1:
        if (m_data.goto1.isSet()) {
            executeGoToReference("(Message)", m_data.goto1);
        }
        break;

     case MessageActionPanel::GoTo2:
        if (m_data.goto2.isSet()) {
            executeGoToReference("(Message)", m_data.goto2);
        }
        break;

     case MessageActionPanel::Reply:
        break;

     case MessageActionPanel::Confirm:
        m_proxy.performMessageAction(game::msg::Mailbox::ToggleConfirmed);
        break;

     case MessageActionPanel::Edit:
     case MessageActionPanel::Redirect:
     case MessageActionPanel::Delete:
     case MessageActionPanel::Forward:
     case MessageActionPanel::Search:
     case MessageActionPanel::Write:
        break;

     case MessageActionPanel::BrowsePrevious:
        m_proxy.browse(game::msg::Browser::Previous, arg, false);
        break;

     case MessageActionPanel::BrowsePreviousAll:
        m_proxy.browse(game::msg::Browser::Previous, arg, true);
        break;

     case MessageActionPanel::BrowseNext:
        m_proxy.browse(game::msg::Browser::Next, arg, false);
        break;

     case MessageActionPanel::BrowseNextAll:
        m_proxy.browse(game::msg::Browser::Next, arg, true);
        break;

     case MessageActionPanel::BrowseFirst:
        m_proxy.browse(game::msg::Browser::First, arg, false);
        break;

     case MessageActionPanel::BrowseFirstAll:
        m_proxy.browse(game::msg::Browser::First, arg, true);
        break;

     case MessageActionPanel::BrowseLast:
        m_proxy.browse(game::msg::Browser::Last, arg, false);
        break;

     case MessageActionPanel::BrowseLastAll:
        m_proxy.browse(game::msg::Browser::Last, arg, true);
        break;

     case MessageActionPanel::BrowseNth:
        if (arg > 0) {
            m_proxy.setCurrentMessage(arg-1);
        }
        break;

     case MessageActionPanel::SearchNext:
     case MessageActionPanel::WriteAll:
     case MessageActionPanel::ReplyAll:
        break;

     case MessageActionPanel::BrowseSubjects:
        doSubjectListDialog(m_proxy, root(), interface().gameSender(), translator());
        break;
    }
}
