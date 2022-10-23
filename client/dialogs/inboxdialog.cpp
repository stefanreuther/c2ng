/**
  *  \file client/dialogs/inboxdialog.cpp
  */

#include "client/dialogs/inboxdialog.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/messageeditor.hpp"
#include "client/dialogs/messagereceiver.hpp"
#include "client/dialogs/sessionfileselectiondialog.hpp"
#include "client/dialogs/subjectlist.hpp"
#include "client/widgets/decayingmessage.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/playersetselector.hpp"
#include "game/actions/preconditions.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/proxy/outboxproxy.hpp"
#include "game/proxy/playerproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"
#include "util/stringparser.hpp"
#include "util/unicodechars.hpp"

using afl::string::Format;
using client::widgets::MessageActionPanel;

namespace {
    void addStatus(ui::rich::Document& doc, const char*const icon, util::SkinColor::Color color, String_t text)
    {
        doc.addParagraph();
        doc.add(util::rich::Text(icon).withColor(color));
        doc.add(" ");
        doc.add(text);
    }
}

/****************************** InboxDialog ******************************/

client::dialogs::InboxDialog::InboxDialog(String_t title, util::RequestSender<game::proxy::MailboxAdaptor> sender, client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx)
    : Control(iface),
      m_link(root, tx),
      m_title(title),
      m_state(),
      m_data(),
      m_outputState(),
      m_loop(root),
      m_actionPanel(root, tx),
      m_content(root.provider().getFont(gfx::FontRequest().setStyle(1))->getCellSize().scaledBy(41, 22), 0, root.provider()),
      m_searchText(),
      m_configProxy(iface.gameSender()),
      m_proxy(sender, root.engine().dispatcher())
{
    m_proxy.sig_update.add(this, &InboxDialog::onUpdate);
    m_proxy.sig_searchFailure.add(this, &InboxDialog::onSearchFailure);
    m_content.sig_linkClick.add(this, &InboxDialog::onLinkClick);
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

    m_searchText = m_configProxy.getOption(m_link, game::config::UserConfiguration::Messages_LastSearch);

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
client::dialogs::InboxDialog::handleScanKeyboardMode(client::si::RequestLink2 link)
{
    defaultHandleScanKeyboardMode(link);
}

void
client::dialogs::InboxDialog::handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetView(link, name, withKeymap);
}

void
client::dialogs::InboxDialog::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymap(link, name, prefix);
}

void
client::dialogs::InboxDialog::handleOverlayMessage(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessage(link, text);
}

game::interface::ContextProvider*
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
    m_actionPanel.setPosition(Format("%d/%d", index+1, m_state.numMessages), msg.isFiltered);
    m_state.currentMessage = index;

    // Buttons
    updateButton(MessageActionPanel::GoTo1, msg.goto1Name);
    updateButton(MessageActionPanel::GoTo2, msg.goto2Name);
    updateButton(MessageActionPanel::Reply, msg.replyName);

    if (msg.actions.contains(game::msg::Mailbox::ToggleConfirmed)) {
        m_actionPanel.enableAction(MessageActionPanel::Confirm, String_t());
    } else {
        m_actionPanel.disableAction(MessageActionPanel::Confirm);
    }

    if (msg.dataStatus == game::proxy::MailboxProxy::DataReceivable) {
        m_actionPanel.enableAction(MessageActionPanel::Accept, String_t());
    } else {
        m_actionPanel.disableAction(MessageActionPanel::Accept);
    }

    // Content
    afl::string::Translator& tx = translator();
    ui::rich::Document& doc = m_content.getDocument();
    doc.clear();
    doc.add(msg.text);
    switch (msg.dataStatus) {
     case game::proxy::MailboxProxy::NoData:
        break;
     case game::proxy::MailboxProxy::DataReceivable:
        addStatus(doc, UTF_RIGHT_POINTER, util::SkinColor::Green, tx("Data can be received"));
        break;
     case game::proxy::MailboxProxy::DataReceived:
        addStatus(doc, UTF_CHECK_MARK, util::SkinColor::Green, tx("Data has been received"));
        break;
     case game::proxy::MailboxProxy::DataExpired:
        addStatus(doc, UTF_BALLOT_CROSS, util::SkinColor::Yellow, tx("Data is expired"));
        break;
     case game::proxy::MailboxProxy::DataWrongPasscode:
        addStatus(doc, UTF_BALLOT_CROSS, util::SkinColor::Red, tx("Wrong passcode"));
        break;
     case game::proxy::MailboxProxy::DataWrongChecksum:
        addStatus(doc, UTF_BALLOT_CROSS, util::SkinColor::Red, tx("Checksum error"));
        break;
     case game::proxy::MailboxProxy::DataFailed:
        addStatus(doc, UTF_BALLOT_CROSS, util::SkinColor::Red, tx("Data error"));
        break;
    }
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
            executeGoToReferenceWait("(Message)", m_data.goto1);
        }
        break;

     case MessageActionPanel::GoTo2:
        if (m_data.goto2.isSet()) {
            executeGoToReferenceWait("(Message)", m_data.goto2);
        }
        break;

     case MessageActionPanel::Reply:
        if (!m_data.reply.empty()) {
            doReply(m_data.reply);
        }
        break;

     case MessageActionPanel::Confirm:
        m_proxy.performMessageAction(game::msg::Mailbox::ToggleConfirmed);
        break;

     case MessageActionPanel::Accept:
        m_proxy.receiveData();
        break;

     case MessageActionPanel::Edit:
     case MessageActionPanel::Redirect:
     case MessageActionPanel::Delete:
        break;

     case MessageActionPanel::Forward:
        doForward();
        break;

     case MessageActionPanel::Search:
        doSearch();
        break;

     case MessageActionPanel::Write:
        doWrite(false);
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
        if (m_searchText.empty()) {
            doSearch();
        } else {
            doSearchNext();
        }
        break;

     case MessageActionPanel::WriteAll:
        doWrite(true);
        break;

     case MessageActionPanel::ReplyAll:
        if (!m_data.replyAll.empty()) {
            doReply(m_data.replyAll);
        }
        break;

     case MessageActionPanel::BrowseSubjects:
        doSubjectListDialog(m_proxy, root(), interface().gameSender(), translator());
        break;
    }
}

void
client::dialogs::InboxDialog::doSearch()
{
    // ex WMessageDisplay::doSearch
    afl::string::Translator& tx = translator();
    ui::widgets::InputLine input(1000, 30, root());
    input.setText(m_searchText);
    if (input.doStandardDialog(tx("Search in messages"), tx("Search for:"), tx)) {
        m_searchText = input.getText();
        if (!m_searchText.empty()) {
            m_configProxy.setOption(game::config::UserConfiguration::Messages_LastSearch, m_searchText);
            m_proxy.search(game::msg::Browser::First, 0, true, m_searchText);
            // Will update the message or call onSearchFailure
        }
    }
}

void
client::dialogs::InboxDialog::doSearchNext()
{
    // ex WMessageDisplay::doSearchNext (sort-of)
    m_proxy.search(game::msg::Browser::Next, 1, true, m_searchText);
    // Will update the message or call onSearchFailure
}

void
client::dialogs::InboxDialog::onSearchFailure()
{
    afl::string::Translator& tx = translator();
    ui::dialogs::MessageBox(tx("Search text not found."), tx("Search in messages"), root())
        .doOkDialog(tx);
}

void
client::dialogs::InboxDialog::doWrite(bool all)
{
    afl::string::Translator& tx = translator();
    String_t heading = all ? tx("Save All Messages") : tx("Save this Message");
    SessionFileSelectionDialog dlg(root(), tx, interface().gameSender(), heading);
    dlg.setPattern(util::FileNamePattern::getAllFilesWithExtensionPattern("txt"));
    if (dlg.runDefault(m_link)) {
        // Do it!
        const String_t fileName = dlg.getResult();
        String_t err;
        bool ok = all
            ? m_proxy.write(m_link, fileName, 0,                      m_state.numMessages,      err)
            : m_proxy.write(m_link, fileName, m_state.currentMessage, m_state.currentMessage+1, err);
        if (!ok) {
            ui::dialogs::MessageBox(Format(tx("Unable to write to file %s: %s"), fileName, err), heading, root())
                .doOkDialog(tx);
        }
    }
}

void
client::dialogs::InboxDialog::onLinkClick(String_t str)
{
    util::StringParser p(str);
    int x, y;
    if (p.parseInt(x) && p.parseCharacter(',') && p.parseInt(y) && p.parseEnd()) {
        executeGoToReferenceWait("(Message)", game::map::Point(x, y));
    }
}

void
client::dialogs::InboxDialog::doForward()
{
    // WMessageDisplay::doForwardMessage()
    afl::string::Translator& tx = translator();

    // Get player data
    game::proxy::PlayerProxy proxy(interface().gameSender());
    game::PlayerArray<String_t> names = proxy.getPlayerNames(m_link, game::Player::ShortName);
    game::PlayerSet_t players = proxy.getAllPlayers(m_link);

    // Player selector
    client::widgets::HelpWidget help(root(), tx, interface().gameSender(), "pcc2:msgin");
    client::widgets::PlayerSetSelector setSelect(root(), names, players + 0, tx);
    MessageReceiver dlg(tx("Forward Message"), setSelect, root(), tx);
    dlg.addUniversalToggle(players);
    dlg.addHelp(help);
    dlg.pack();
    root().moveWidgetToEdge(dlg, gfx::RightAlign, gfx::BottomAlign, 10);

    if (dlg.run() != 0) {
        // Fetch message parameters
        game::proxy::MailboxProxy::QuoteResult qm = m_proxy.quoteMessage(m_link, m_state.currentMessage, game::proxy::MailboxProxy::QuoteForForwarding);

        // Prepare message editor
        game::proxy::OutboxProxy outProxy(interface().gameSender());
        MessageEditor ed(root(), outProxy, interface().gameSender(), tx);
        ed.setSender(qm.sender);
        ed.setReceivers(setSelect.getSelectedPlayers());
        ed.setText(qm.text);
        if (ed.run()) {
            outProxy.addMessage(ed.getSender(), ed.getText(), ed.getReceivers());
        }
    }
}

void
client::dialogs::InboxDialog::doReply(game::PlayerSet_t to)
{
    // WMessageActionPanel::doReply()
    // Fetch message parameters
    game::proxy::MailboxProxy::QuoteResult qm = m_proxy.quoteMessage(m_link, m_state.currentMessage, game::proxy::MailboxProxy::QuoteForReplying);

    // Prepare message editor
    game::proxy::OutboxProxy outProxy(interface().gameSender());
    MessageEditor ed(root(), outProxy, interface().gameSender(), translator());
    ed.setSender(qm.sender);
    ed.setReceivers(to);
    ed.setText(qm.text);
    if (ed.run()) {
        outProxy.addMessage(ed.getSender(), ed.getText(), ed.getReceivers());
    }
}
