/**
  *  \file client/dialogs/outboxdialog.cpp
  *  \brief Class client::dialogs::OutboxDialog
  *
  *  FIXME: this is a very close relative of InboxDialog. Can we merge?
  */

#include "client/dialogs/outboxdialog.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/messageeditor.hpp"
#include "client/dialogs/messagereceiver.hpp"
#include "client/dialogs/subjectlist.hpp"
#include "client/widgets/decayingmessage.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/playersetselector.hpp"
#include "game/actions/preconditions.hpp"
#include "game/proxy/playerproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"

using client::widgets::MessageActionPanel;

client::dialogs::OutboxDialog::OutboxDialog(String_t title, client::si::UserSide& iface, ui::Root& root, String_t helpPage, afl::string::Translator& tx)
    : Control(iface),
      m_link(root, tx),
      m_title(title),
      m_state(),
      m_data(),
      m_outputState(),
      m_loop(root),
      m_actionPanel(root, tx),
      m_content(root.provider().getFont(gfx::FontRequest().setStyle(1))->getCellSize().scaledBy(41, 22), 0, root.provider()),
      m_helpPage(helpPage),
      m_outboxProxy(iface.gameSender()),
      m_proxy(m_outboxProxy.getMailboxAdaptor(), root.engine().dispatcher())
{
    m_proxy.sig_update.add(this, &OutboxDialog::onUpdate);
}

client::dialogs::OutboxDialog::~OutboxDialog()
{ }

bool
client::dialogs::OutboxDialog::run(client::si::OutputState& out,
                                   String_t noMessageAdvice)
{
    // Initialize messenger
    // ex WOutboxWindow::init (sort-of)
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

    client::widgets::HelpWidget help(root(), translator(), interface().gameSender(), m_helpPage);

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
    m_actionPanel.sig_action.add(this, &OutboxDialog::onAction);

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
client::dialogs::OutboxDialog::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    dialogHandleStateChange(link, target, m_outputState, m_loop, 1);
}

void
client::dialogs::OutboxDialog::handleEndDialog(client::si::RequestLink2 link, int code)
{
    dialogHandleEndDialog(link, code, m_outputState, m_loop, 1);
}

void
client::dialogs::OutboxDialog::handlePopupConsole(client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
client::dialogs::OutboxDialog::handleScanKeyboardMode(client::si::RequestLink2 link)
{
    defaultHandleScanKeyboardMode(link);
}

void
client::dialogs::OutboxDialog::handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetView(link, name, withKeymap);
}

void
client::dialogs::OutboxDialog::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymap(link, name, prefix);
}

void
client::dialogs::OutboxDialog::handleOverlayMessage(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessage(link, text);
}

afl::base::Optional<game::Id_t>
client::dialogs::OutboxDialog::getFocusedObjectId(game::Reference::Type type) const
{
    return defaultGetFocusedObjectId(type);
}

game::interface::ContextProvider*
client::dialogs::OutboxDialog::createContextProvider()
{
    return 0;
}


/*
 *  OutboxDialog Methods
 */

void
client::dialogs::OutboxDialog::onUpdate(size_t index, const game::proxy::MailboxProxy::Message& msg)
{
    // ex WMessageActionPanel::onBrowse (sort-of)
    m_data = msg;

    // Position
    m_actionPanel.setPosition(afl::string::Format("%d/%d", index+1, m_state.numMessages), msg.isFiltered);

    // Buttons
    updateButton(MessageActionPanel::GoTo1, msg.goto1Name);
    updateButton(MessageActionPanel::GoTo2, msg.goto2Name);
    m_actionPanel.enableAction(MessageActionPanel::Redirect, String_t());
    m_actionPanel.enableAction(MessageActionPanel::Delete, String_t());
    m_actionPanel.enableAction(MessageActionPanel::Edit, String_t());

    // Content
    ui::rich::Document& doc = m_content.getDocument();
    doc.clear();
    doc.add(msg.text);
    doc.finish();
    m_content.handleDocumentUpdate();
}

void
client::dialogs::OutboxDialog::updateButton(client::widgets::MessageActionPanel::Action a, const String_t& s)
{
    if (s.empty()) {
        m_actionPanel.disableAction(a);
    } else {
        m_actionPanel.enableAction(a, s);
    }
}

void
client::dialogs::OutboxDialog::onAction(client::widgets::MessageActionPanel::Action a, int arg)
{
    switch (a) {
     case MessageActionPanel::GoTo1:
        if (m_data.goto1.isSet()) {
            executeGoToReferenceWait("(Message)", m_data.goto1, ShowUnit);
        }
        break;

     case MessageActionPanel::GoTo2:
        if (m_data.goto2.isSet()) {
            executeGoToReferenceWait("(Message)", m_data.goto2, ShowUnit);
        }
        break;

     case MessageActionPanel::Reply:
        break;

     case MessageActionPanel::Confirm:
        m_proxy.performMessageAction(game::msg::Mailbox::ToggleConfirmed);  // Not needed here
        break;

     case MessageActionPanel::Edit:
        editMessage();
        break;

     case MessageActionPanel::Accept:
        break;

     case MessageActionPanel::Redirect:
        redirectMessage();
        break;

     case MessageActionPanel::Delete:
        deleteMessage();
        break;

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

void
client::dialogs::OutboxDialog::editMessage()
{
    // Fetch message
    Downlink ind(root(), translator());
    game::proxy::OutboxProxy::Info info;
    game::Id_t id = m_data.id;
    if (m_outboxProxy.getMessage(ind, id, info)) {
        // Editor
        MessageEditor ed(root(), m_outboxProxy, interface().gameSender(), translator());
        ed.setText(info.text);
        ed.setReceivers(info.receivers);
        ed.setSender(info.sender);
        if (ed.run()) {
            m_outboxProxy.setMessageReceivers(id, ed.getReceivers());
            m_outboxProxy.setMessageText(id, ed.getText());
            reload();
        }
    }
}

void
client::dialogs::OutboxDialog::redirectMessage()
{
    // Fetch message
    Downlink ind(root(), translator());
    game::proxy::OutboxProxy::Info info;
    game::Id_t id = m_data.id;
    if (m_outboxProxy.getMessage(ind, id, info)) {
        // Data
        game::proxy::PlayerProxy proxy(interface().gameSender());
        game::PlayerArray<String_t> names = proxy.getPlayerNames(ind, game::Player::ShortName);
        game::PlayerSet_t players = proxy.getAllPlayers(ind);

        // Widgets
        client::widgets::HelpWidget help(root(), translator(), interface().gameSender(), m_helpPage);
        client::widgets::PlayerSetSelector setSelect(root(), names, players + 0, translator());
        setSelect.setSelectedPlayers(info.receivers);
        MessageReceiver dlg(m_title, setSelect, root(), translator());
        dlg.addUniversalToggle(players);
        dlg.addHelp(help);

        dlg.pack();
        root().centerWidget(dlg);
        if (dlg.run() != 0) {
            m_outboxProxy.setMessageReceivers(id, setSelect.getSelectedPlayers());
            reload();
        }
    }
}

void
client::dialogs::OutboxDialog::deleteMessage()
{
    afl::string::Translator& tx = translator();
    if (ui::dialogs::MessageBox(tx("Delete this message?"), tx("Revise Messages"), root()).doYesNoDialog(tx)) {
        m_outboxProxy.deleteMessage(m_data.id);
        reload();
    }
}

void
client::dialogs::OutboxDialog::reload()
{
    // FIXME: this should be automatic, but currently, Mailbox has no change notification
    m_proxy.getStatus(m_link, m_state);
    m_proxy.setCurrentMessage(m_state.currentMessage);
    if (m_state.numMessages == 0) {
        // No more messages, close dialog
        m_loop.stop(0);
    }
}
