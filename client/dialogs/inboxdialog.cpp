/**
  *  \file client/dialogs/inboxdialog.cpp
  */

#include "client/dialogs/inboxdialog.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/spacer.hpp"
#include "ui/window.hpp"
#include "game/parser/format.hpp"

namespace {
    using client::widgets::MessageActionPanel;

    /* @q CCUI$CurrentInMsg (Internal Variable)
       Zero-based index of current inbox message.
       @since PCC2 2.40.4 */
    const char*const INDEX_VAR_NAME = "CCUI$CURRENTINMSG";

    game::msg::Inbox* getInbox(game::Session& session)
    {
        if (game::Game* g = session.getGame().get()) {
            if (game::Turn* t = g->getViewpointTurn().get()) {
                return &t->inbox();
            }
        }
        return 0;
    }

    bool isFiltered(game::Session& /*session*/, game::msg::Inbox& /*inbox*/, size_t /*index*/)
    {
        // FIXME: implement me (isFiltered)
        return false;
    }

    size_t findFirstMessage(game::Session& session, game::msg::Inbox& inbox)
    {
        size_t numMessages = inbox.getNumMessages();
        size_t i = 0;
        while (i < numMessages && isFiltered(session, inbox, i)) {
            ++i;
        }
        if (i >= numMessages) {
            i = 0;
        }
        return i;
    }

    size_t findLastMessage(game::Session& session, game::msg::Inbox& inbox)
    {
        size_t numMessages = inbox.getNumMessages();
        size_t i = numMessages;
        while (i > 0 && isFiltered(session, inbox, i-1)) {
            --i;
        }
        if (i == 0) {
            i = numMessages;
        }
        return i-1;
    }
}

void
client::dialogs::InboxDialog::State::load(game::Session& session, size_t index)
{
    game::msg::Inbox* inbox = getInbox(session);
    game::Root* root = session.getRoot().get();
    if (!inbox || !root) {
        return;
    }

    game::parser::Format fmt;
    formatMessage(fmt,
                  inbox->getMessageText(index, session.translator(), root->playerList()),
                  root->playerList());

    this->limit = inbox->getNumMessages();
    this->current = index;
    this->dim = isFiltered(session, *inbox, index);
    // goto1, goto1name
    this->goto2 = fmt.firstLink;
    session.getReferenceName(fmt.firstLink, this->goto2Name);
    this->reply = fmt.reply;
    this->replyAll = fmt.replyAll;
    if (!fmt.reply.empty()) {
        this->replyName = formatPlayerHostSet(fmt.reply, root->playerList(), session.translator());
    }
    this->text = fmt.text.withStyle(util::rich::StyleAttribute::Fixed);

    // Remember that we're here
    session.world().setNewGlobalValue(INDEX_VAR_NAME, interpreter::makeIntegerValue(int32_t(index)));
}

/*
 *  Base class for queries
 *
 *  Most operations we perform eventually load a new message for display.
 *  This implements the boilerplate for doing that: do
 *     MyClass(..).call(theInboxDialog)
 *  to switch theInboxDialog to a new message.
 */

class client::dialogs::InboxDialog::Query : public util::Request<game::Session> {
 public:
    virtual void handle(game::Session& s, game::msg::Inbox& inbox, State& out) = 0;

    virtual void handle(game::Session& s)
        {
            if (game::msg::Inbox* p = getInbox(s)) {
                handle(s, *p, m_state);
            }
        }
    void call(InboxDialog& parent)
        {
            parent.m_link.call(parent.interface().gameSender(), *this);
            parent.setState(m_state);
        }
 private:
    State m_state;
};

/*
 *  InitQuery: find message to show first
 *
 *  Like all queries, this also provides the total message count and thus serves
 *  as the decision whether to show the messenger dialog at all.
 */

class client::dialogs::InboxDialog::InitQuery : public Query {
 public:
    virtual void handle(game::Session& s, game::msg::Inbox& inbox, State& state)
        {
            size_t numMessages = 0;
            size_t currentMessage = 0;
            bool haveCurrentMessage = false;

            // Get number of messages
            numMessages = inbox.getNumMessages();

            // Get cursor
            try {
                int32_t i;
                if (interpreter::checkIntegerArg(i, s.world().getGlobalValue(INDEX_VAR_NAME))) {
                    currentMessage = static_cast<size_t>(i);
                    if (currentMessage < numMessages) {
                        haveCurrentMessage = true;
                    }
                }
            }
            catch (...)
            { }

            // Postprocess
            if (!haveCurrentMessage) {
                currentMessage = findFirstMessage(s, inbox);
            }

            // Finish
            state.load(s, currentMessage);
        }
};

/*
 *  BrowseQuery: browse to next/previous message
 */

class client::dialogs::InboxDialog::BrowseQuery : public Query {
 public:
    BrowseQuery(size_t current, bool forward, bool acceptFiltered, int amount)
        : m_current(current), m_forward(forward), m_acceptFiltered(acceptFiltered), m_amount(amount)
        { }
    void handle(game::Session& s, game::msg::Inbox& inbox, State& state)
        {
            bool filtered = m_acceptFiltered || isFiltered(s, inbox, m_current);
            int amount = std::max(1, m_amount);
            bool found = false;
            size_t current = m_current;
            if (m_forward) {
                // ex WMessageDisplay::doNext
                while (!found && ++current < inbox.getNumMessages()) {
                    if (filtered || !isFiltered(s, inbox, current)) {
                        if (--amount == 0) {
                            found = true;
                        }
                    }
                }
            } else {
                // ex WMessageDisplay::doPrev
                while (!found && current > 0) {
                    --current;
                    if (filtered || !isFiltered(s, inbox, current)) {
                        if (--amount == 0) {
                            found = true;
                        }
                    }
                }
            }

            // Load message
            state.load(s, found ? current : m_current);
        }
 private:
    const size_t m_current;
    const bool m_forward;
    const bool m_acceptFiltered;
    const int m_amount;
};

/*
 *  FirstQuery: go to first unfiltered message
 */

class client::dialogs::InboxDialog::FirstQuery : public Query {
 public:
    void handle(game::Session& s, game::msg::Inbox& inbox, State& state)
        { state.load(s, findFirstMessage(s, inbox)); }
};

/*
 *  FirstQuery: go to last unfiltered message
 */

class client::dialogs::InboxDialog::LastQuery : public Query {
 public:
    void handle(game::Session& s, game::msg::Inbox& inbox, State& state)
        { state.load(s, findLastMessage(s, inbox)); }
};

/*
 *  LoadQuery: load a message by (0-based) index
 */

class client::dialogs::InboxDialog::LoadQuery : public Query {
 public:
    LoadQuery(size_t index)
        : m_index(index)
        { }
    void handle(game::Session& s, game::msg::Inbox& inbox, State& state)
        { state.load(s, std::min(m_index, inbox.getNumMessages()-1)); }
 private:
    size_t m_index;
};

/****************************** InboxDialog ******************************/

client::dialogs::InboxDialog::InboxDialog(client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx)
    : Control(iface, root, tx),
      m_root(root),
      m_link(root),
      m_state(),
      m_outputState(),
      m_loop(root),
      m_actionPanel(root),
      m_content(root.provider().getFont(gfx::FontRequest().setStyle(1))->getCellSize().scaledBy(41, 22), 0, root.provider())
{ }

client::dialogs::InboxDialog::~InboxDialog()
{ }

void
client::dialogs::InboxDialog::run(client::si::OutputState& out)
{
    // Initialize messenger
    InitQuery().call(*this);
    if (m_state.limit == 0) {
        return;
    }

    // Window
    //   HBox
    //     VBox
    //       Actions
    //       HBox
    //         Close
    //         Spacer
    //   Content

    ui::Window win("!Messages", m_root.provider(), m_root.colorScheme(), ui::BLUE_BLACK_WINDOW, ui::layout::HBox::instance5);
    ui::Group g1(ui::layout::VBox::instance5);
    g1.add(m_actionPanel);

    ui::Group g12(ui::layout::HBox::instance5);
    ui::widgets::Button btnOK("!OK", util::Key_Escape, m_root);
    ui::Spacer spc;
    ui::PrefixArgument prefix(m_root);
    g12.add(btnOK);
    g12.add(spc);
    g1.add(g12);
    win.add(g1);
    win.add(m_content);
    win.add(prefix);

    btnOK.sig_fire.addNewClosure(m_loop.makeStop(0));
    m_actionPanel.sig_action.add(this, &InboxDialog::onAction);

    win.pack();

    // Reload state after pack() to format content with correct width
    setState(m_state);

    m_root.centerWidget(win);
    m_root.add(win);

    // Run (this will immediately exit if one of the above scripts requested a context change.)
    m_loop.run();

    out = m_outputState;
}

/*
 *  Control methods
 */

void
client::dialogs::InboxDialog::handleStateChange(client::si::UserSide& us, client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    using client::si::OutputState;
    switch (target) {
     case OutputState::NoChange:
        us.continueProcess(link);
        break;

     case OutputState::PlayerScreen:
     case OutputState::ExitProgram:
     case OutputState::ExitGame:
     case OutputState::ShipScreen:
     case OutputState::PlanetScreen:
     case OutputState::BaseScreen:
        us.detachProcess(link);
        m_outputState.set(link, target);
        m_loop.stop(0);
        break;
    }
}

void
client::dialogs::InboxDialog::handleEndDialog(client::si::UserSide& us, client::si::RequestLink2 link, int /*code*/)
{
    us.detachProcess(link);
    m_outputState.set(link, client::si::OutputState::NoChange);
    m_loop.stop(0);
}

void
client::dialogs::InboxDialog::handlePopupConsole(client::si::UserSide& us, client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(us, link);
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
client::dialogs::InboxDialog::setState(const State& state)
{
    // ex WMessageActionPanel::onBrowse (sort-of)
    m_state = state;

    // Position
    if (m_state.limit > 0) {
        m_actionPanel.setPosition(afl::string::Format("%d/%d", m_state.current+1, m_state.limit), m_state.dim);
    } else {
        m_actionPanel.setPosition(String_t(), false);
    }

    // Buttons
    updateButton(MessageActionPanel::GoTo1, m_state.goto1Name);
    updateButton(MessageActionPanel::GoTo2, m_state.goto2Name);
    updateButton(MessageActionPanel::Reply, m_state.replyName);

    // Content
    ui::rich::Document& doc = m_content.getDocument();
    doc.clear();
    doc.add(m_state.text);
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
     case MessageActionPanel::GoTo2:
     case MessageActionPanel::Reply:
     case MessageActionPanel::Confirm:
     case MessageActionPanel::Edit:
     case MessageActionPanel::Redirect:
     case MessageActionPanel::Delete:
     case MessageActionPanel::Forward:
     case MessageActionPanel::Search:
     case MessageActionPanel::Write:
        break;

     case MessageActionPanel::BrowsePrevious:
        BrowseQuery(m_state.current, false, false, arg).call(*this);
        break;

     case MessageActionPanel::BrowsePreviousAll:
        BrowseQuery(m_state.current, false, true, arg).call(*this);
        break;

     case MessageActionPanel::BrowseNext:
        BrowseQuery(m_state.current, true, true, arg).call(*this);
        break;

     case MessageActionPanel::BrowseNextAll:
        BrowseQuery(m_state.current, true, true, arg).call(*this);
        break;

     case MessageActionPanel::BrowseFirst:
        FirstQuery().call(*this);
        break;

     case MessageActionPanel::BrowseFirstAll:
        LoadQuery(0).call(*this);
        break;

     case MessageActionPanel::BrowseLast:
        LastQuery().call(*this);
        break;

     case MessageActionPanel::BrowseLastAll:
        LoadQuery(m_state.current-1).call(*this);
        break;

     case MessageActionPanel::BrowseNth:
        if (arg > 0) {
            LoadQuery(arg-1).call(*this);
        }
        break;

     case MessageActionPanel::SearchNext:
     case MessageActionPanel::WriteAll:
     case MessageActionPanel::ReplyAll:
        break;
    }
}
