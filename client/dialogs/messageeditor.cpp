/**
  *  \file client/dialogs/messageeditor.cpp
  *  \brief Class client::dialogs::MessageEditor
  */

#include "client/dialogs/messageeditor.hpp"
#include "afl/base/deleter.hpp"
#include "client/dialogs/messagereceiver.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/playersetselector.hpp"
#include "game/proxy/playerproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/editor.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"
#include "util/syntax/highlighter.hpp"
#include "util/syntax/segment.hpp"

using ui::dialogs::MessageBox;

namespace {
    /*
     *  Message Dimensions
     */

    // Maximum line length (limit from planets.exe)
    const size_t MAX_LINE_LENGTH = 40;

    // Maximum number of stored lines; some invisible extra lines to not immediately lose information scrolling out
    const size_t MAX_STORED_LINES = 24;

    // Maximum lines in a message, including headers
    const size_t MAX_MESSAGE_LINES = 20;


    /*
     *  Highlighting
     */

    class MessageHighlighter : public util::syntax::Highlighter {
     public:
        virtual void init(afl::string::ConstStringMemory_t text)
            { m_text = text; }
        virtual bool scan(util::syntax::Segment& result)
            {
                // End?
                if (m_text.empty()) {
                    return false;
                }

                // Check first non-blank
                size_t index = 0;
                const char* first;
                while ((first = m_text.at(index)) != 0 && *first == ' ') {
                    ++index;
                }
                util::syntax::Format fmt = (first == 0
                                            ? util::syntax::DefaultFormat
                                            : *first == '>'
                                            ? util::syntax::QuoteFormat
                                            : *first == '<'
                                            ? util::syntax::SectionFormat
                                            : util::syntax::DefaultFormat);
                result.set(fmt, m_text);
                m_text.reset();
                return true;
            }
     private:
        afl::string::ConstStringMemory_t m_text;
    };


    /*
     *  Character filter
     */

    class CharacterFilter : public ui::widgets::Editor::CharacterFilter_t {
     public:
        CharacterFilter(game::StringVerifier* verifier)
            : m_verifier(verifier)
            { }
        bool call(afl::charset::Unichar_t ch)
            {
                return m_verifier.get() == 0
                    || m_verifier->isValidCharacter(game::StringVerifier::Message, ch);
            }
     private:
        std::auto_ptr<game::StringVerifier> m_verifier;
    };
}

client::dialogs::MessageEditor::MessageEditor(ui::Root& root, game::proxy::OutboxProxy& proxy, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
    : m_root(root),
      m_proxy(proxy),
      m_gameSender(gameSender),
      m_translator(tx),
      m_title(tx("Edit Message")),
      m_editor(),
      m_loop(root),
      m_receivers(),
      m_sender(0),
      m_numHeaderLines()
{ }

client::dialogs::MessageEditor::~MessageEditor()
{ }

void
client::dialogs::MessageEditor::setTitle(String_t title)
{
    m_title = title;
}

void
client::dialogs::MessageEditor::setText(String_t text)
{
    // ex WEditMessageWindow::setText
    for (size_t i = m_numHeaderLines; i < MAX_STORED_LINES; ++i) {
        size_t pos = text.find('\n');
        if (pos != String_t::npos) {
            m_editor.setLine(i, text.substr(0, pos));
            text.erase(0, pos+1);
        } else {
            m_editor.setLine(i, text);
            text.clear();
        }
    }
}

void
client::dialogs::MessageEditor::setReceivers(game::PlayerSet_t receivers)
{
    // ex WEditMessageWindow::setReceivers (part)
    m_receivers = receivers;
}

void
client::dialogs::MessageEditor::setSender(int sender)
{
    m_sender = sender;
}

String_t
client::dialogs::MessageEditor::getText() const
{
    return m_editor.getRange(m_numHeaderLines, 0, MAX_MESSAGE_LINES, 0);
}

game::PlayerSet_t
client::dialogs::MessageEditor::getReceivers() const
{
    return m_receivers;
}

int
client::dialogs::MessageEditor::getSender() const
{
    return m_sender;
}

bool
client::dialogs::MessageEditor::run()
{
    // ex WEditMessageWindow::WEditMessageWindow (sort-of), editor.inc:EditMessage

    // Update content
    // Done here so other methods needn't block.
    Downlink ind(m_root, m_translator);
    updateContent(ind);

    // Window
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_title, m_root.provider(), m_root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::VBox::instance5));

    // Editor
    MessageHighlighter highl;
    CharacterFilter filter(m_proxy.createStringVerifier(ind));
    ui::widgets::Editor& editor = del.addNew(new ui::widgets::Editor(m_editor, m_root));
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, editor));
    editor.setPreferredSizeInCells(MAX_LINE_LENGTH, MAX_MESSAGE_LINES);
    editor.setHighlighter(&highl);

    // Buttons
    ui::widgets::Button& btnHelp   = del.addNew(new ui::widgets::Button(m_translator("Help"),   'h',              m_root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(m_translator("Cancel"), util::Key_Escape, m_root));
    ui::widgets::Button& btnSend   = del.addNew(new ui::widgets::Button(m_translator("Send"),   util::Key_F10,    m_root));
    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(btnHelp);
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnCancel);
    g.add(btnSend);
    win.add(g);

    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:msgout"));
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    // Hot-keys
    // FIXME: missing PCC1 functionality:
    //   Alt-S   save-as-template (conflict with send!)
    //   Ctrl-W  save-to-file
    //   Ctrl-R  load-from-file
    //   PgUp    first line, then home
    //   PgDn    last line, then end
    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    disp.add(util::Key_Return + util::KeyMod_Ctrl, this, &MessageEditor::onSend);
    disp.add('s'              + util::KeyMod_Alt, this, &MessageEditor::onSend);
    disp.add('t'              + util::KeyMod_Alt, this, &MessageEditor::onChangeReceivers);
    win.add(disp);

    // Actions
    btnHelp.dispatchKeyTo(help);
    btnCancel.sig_fire.add(this, &MessageEditor::onCancel);
    btnSend.sig_fire.add(this, &MessageEditor::onSend);

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    return m_loop.run() != 0;
}

void
client::dialogs::MessageEditor::onCancel()
{
    if (MessageBox(m_translator("Discard changes?"), m_title, m_root).doYesNoDialog(m_translator)) {
        m_loop.stop(0);
    }
}

void
client::dialogs::MessageEditor::onSend()
{
    if (MessageBox(m_translator("Send this message?"), m_title, m_root).doYesNoDialog(m_translator)) {
        m_loop.stop(1);
    }
}

void
client::dialogs::MessageEditor::onChangeReceivers()
{
    // ex WEditMessageWindow::handleEvent (part)
    // Data
    game::proxy::PlayerProxy proxy(m_gameSender);
    Downlink ind(m_root, m_translator);
    game::PlayerArray<String_t> names = proxy.getPlayerNames(ind, game::Player::ShortName);
    game::PlayerSet_t players = proxy.getAllPlayers(ind);

    // Widgets
    client::widgets::HelpWidget help(m_root, m_translator, m_gameSender, "pcc2:msgout");
    client::widgets::PlayerSetSelector setSelect(m_root, names, players + 0, m_translator);
    setSelect.setSelectedPlayers(m_receivers);
    MessageReceiver dlg(m_title, setSelect, m_root, m_translator);
    dlg.addUniversalToggle(players);
    dlg.addHelp(help);

    dlg.pack();
    m_root.moveWidgetToEdge(dlg, gfx::RightAlign, gfx::BottomAlign, 10);
    if (dlg.run() != 0) {
        setReceivers(setSelect.getSelectedPlayers());
        updateContent(ind);
    }
}

void
client::dialogs::MessageEditor::updateContent(game::proxy::WaitIndicator& ind)
{
    // ex WEditMessageWindow::setReceivers (part)
    String_t newHeader = m_proxy.getHeadersForDisplay(ind, m_sender, m_receivers);
    size_t line = 0;
    while (!newHeader.empty()) {
        if (line >= m_numHeaderLines) {
            m_editor.insertLine(m_numHeaderLines, 1);
            ++m_numHeaderLines;
        }
        m_editor.setLine(line, afl::string::strFirst(newHeader, "\n"));
        afl::string::strRemove(newHeader, "\n");
        ++line;
    }
    if (line < m_numHeaderLines) {
        m_editor.deleteLine(line, m_numHeaderLines - line);
        m_numHeaderLines = line;
    }
    m_editor.setUserLineLimit(m_numHeaderLines, MAX_MESSAGE_LINES-1);
}
