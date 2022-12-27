/**
  *  \file client/dialogs/scriptcommanddialog.cpp
  *  \brief Script Command Dialog
  */

#include "client/dialogs/scriptcommanddialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/charset/utf8.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/interface/completionlist.hpp"
#include "game/proxy/scripteditorproxy.hpp"
#include "interpreter/taskeditor.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"

using ui::widgets::InputLine;

namespace {
    void insertCompletion(InputLine& input, const String_t& stem, const String_t& completion)
    {
        if (completion.size() > stem.size()) {
            input.setFlag(InputLine::TypeErase, false);
            input.insertText(completion.substr(stem.size()));
        }
    }
}

client::dialogs::ScriptCommandDialog::ScriptCommandDialog(String_t prompt, client::si::UserSide& userSide)
    : m_prompt(prompt),
      m_title(prompt),
      m_help(),
      m_userSide(userSide),
      m_onlyCommands(false),
      m_enforceTask(false),
      m_input(4000, 35, userSide.root()),
      m_loop(userSide.root())
{ }

void
client::dialogs::ScriptCommandDialog::setCommand(String_t cmd)
{
    m_input.setText(cmd);
}

String_t
client::dialogs::ScriptCommandDialog::getCommand() const
{
    return m_input.getText();
}

void
client::dialogs::ScriptCommandDialog::setHelp(String_t help)
{
    m_help = help;
}

void
client::dialogs::ScriptCommandDialog::setTitle(String_t title)
{
    m_title = title;
}

void
client::dialogs::ScriptCommandDialog::setOnlyCommands(bool onlyCommands)
{
    m_onlyCommands = onlyCommands;
}

void
client::dialogs::ScriptCommandDialog::setEnforceTask(bool enforceTask)
{
    m_enforceTask = enforceTask;
}

bool
client::dialogs::ScriptCommandDialog::run()
{
    // ex WEditCommandWindow::init
    ui::Root& root = m_userSide.root();
    afl::string::Translator& tx = m_userSide.translator();

    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_title, root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(del.addNew(new ui::widgets::StaticText(m_prompt, util::SkinColor::Static, "+", root.provider())));
    win.add(m_input);

    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(root, tx));
    win.add(btn);
    btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    btn.ok().sig_fire.add(this, &ScriptCommandDialog::onOK);

    if (!m_help.empty()) {
        ui::Widget& h = del.addNew(new client::widgets::HelpWidget(root, tx, m_userSide.gameSender(), m_help));
        win.add(h);
        btn.addHelp(h);
    }

    win.add(del.addNew(new ui::widgets::Quit(root, m_loop)));
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));
    win.pack();

    root.centerWidget(win);
    root.add(win);
    return m_loop.run() != 0;
}

bool
client::dialogs::ScriptCommandDialog::handleKey(util::Key_t key, int /*prefix*/)
{
    switch (key) {
     case util::Key_Tab:
        doCompletion(m_input, m_userSide, m_onlyCommands);
        return true;

     default:
        return false;
    }
}

void
client::dialogs::ScriptCommandDialog::onOK()
{
    // ex WEditCommandWindow::onOk()
    if (m_enforceTask && !interpreter::TaskEditor::isValidCommand(m_input.getText())) {
        afl::string::Translator& tx = m_userSide.translator();
        ui::dialogs::MessageBox(tx("This is not a valid auto task command."), tx("Error"), m_userSide.root())
            .doOkDialog(tx);
    } else {
        m_loop.stop(1);
    }
}

void
client::dialogs::doCompletion(ui::widgets::InputLine& input, client::si::UserSide& userSide, bool onlyCommands)
{
    // Environment
    afl::string::Translator& tx = userSide.translator();
    ui::Root& root = userSide.root();

    // Retrieve completions
    Downlink link(root, tx);
    game::interface::CompletionList result;
    game::proxy::ScriptEditorProxy(userSide.gameSender())
        .buildCompletionList(link, result,
                             afl::charset::Utf8().substr(input.getText(), 0, input.getCursorIndex()),
                             onlyCommands,
                             std::auto_ptr<game::interface::ContextProvider>(userSide.createContextProvider()));

    // Process
    String_t stem = result.getStem();
    String_t immediate = result.getImmediateCompletion();
    if (immediate.size() > stem.size()) {
        insertCompletion(input, stem, immediate);
    } else if (!result.isEmpty()) {
        ui::widgets::StringListbox list(root.provider(), root.colorScheme());
        int32_t i = 0;
        for (game::interface::CompletionList::Iterator_t it = result.begin(), e = result.end(); it != e; ++it) {
            list.addItem(i, *it);
            ++i;
        }
        list.sortItemsAlphabetically();
        if (list.doStandardDialog(tx("Completions"), String_t(), 0, root, tx) && list.getCurrentKey(i)) {
            game::interface::CompletionList::Iterator_t it = result.begin(), e = result.end();
            while (it != e && i != 0) {
                ++it, --i;
            }
            if (it != e) {
                insertCompletion(input, stem, *it);
            }
        } else {
        }
    } else {
        // no completions available
    }

    // No matter what happened, should still clear TypeErase to avoid new input overwriting old one.
    input.setFlag(InputLine::TypeErase, false);
}
