/**
  *  \file ui/dialogs/messagebox.cpp
  */

#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/quit.hpp"
#include "util/keystring.hpp"
#include "util/rich/styleattribute.hpp"

// Constructor.
ui::dialogs::MessageBox::MessageBox(String_t text, String_t title, Root& root)
    : Window(title, root.provider(), root.colorScheme(), BLUE_WINDOW, ui::layout::VBox::instance5),
      m_deleter(),
      m_buttonGroup(m_deleter.addNew(new Group(ui::layout::HBox::instance5))),
      m_keyDispatcher(m_deleter.addNew(new ui::widgets::KeyDispatcher())),
      m_root(root),
      m_loop(root),
      m_flags(),
      m_firstCommand(0),
      m_lastCommand(0)
{
    // ex UIMessageBox::UIMessageBox
    init(text);
}

// Constructor.
ui::dialogs::MessageBox::MessageBox(util::rich::Text text, String_t title, Root& root)
    : Window(title, root.provider(), root.colorScheme(), BLUE_WINDOW, ui::layout::VBox::instance5),
      m_deleter(),
      m_buttonGroup(m_deleter.addNew(new Group(ui::layout::HBox::instance5))),
      m_keyDispatcher(m_deleter.addNew(new ui::widgets::KeyDispatcher())),
      m_root(root),
      m_loop(root),
      m_flags(),
      m_firstCommand(0),
      m_lastCommand(0)
{
    // ex UIMessageBox::UIMessageBox
    init(text);
}

// Destructor.
ui::dialogs::MessageBox::~MessageBox()
{ }

// Add a button.
ui::dialogs::MessageBox&
ui::dialogs::MessageBox::addButton(int id, String_t text, util::Key_t key)
{
    // ex UIMessageBox::addButton
    ui::widgets::Button& btn = m_deleter.addNew(new ui::widgets::Button(text, key, m_root));
    m_buttonGroup.add(btn);
    btn.sig_fire.addNewClosure(m_loop.makeStop(id));
    checkKey(id, key, true);
    return *this;
}

// Add a button.
ui::dialogs::MessageBox&
ui::dialogs::MessageBox::addButton(int id, const util::KeyString& ks)
{
    return addButton(id, ks.getString(), ks.getKey());
}

// Add a key.
ui::dialogs::MessageBox&
ui::dialogs::MessageBox::addKey(int id, util::Key_t key)
{
    // ex UIMessageBox::addKey
    m_keyDispatcher.addNewClosure(key, m_loop.makeStop(id));
    checkKey(id, key, false);
    return *this;
}

// Ignore a key.
ui::dialogs::MessageBox&
ui::dialogs::MessageBox::ignoreKey(util::Key_t key)
{
    checkKey(0, key, false);
    return *this;
}

// Operate the dialog.
int
ui::dialogs::MessageBox::run()
{
    if (!m_flags.contains(HaveRun)) {
        // Complete the button group
        m_buttonGroup.add(m_deleter.addNew(new Spacer()));
        add(m_deleter.addNew(new ui::widgets::Quit(m_root, m_loop)).withCode(m_lastCommand));

        // Complete keys (ESC must be first because we're overwriting m_lastCommand)
        if (!m_flags.contains(HaveEscape)) {
            addKey(m_lastCommand, util::Key_Escape);
        }
        if (!m_flags.contains(HaveReturn)) {
            addKey(m_firstCommand, util::Key_Return);
        }

        pack();
        m_flags += HaveRun;
    }

    // Do it
    m_root.centerWidget(*this);
    m_root.add(*this);
    int n = m_loop.run();
    m_root.remove(*this);
    return n;
}

// Build and operate a Yes/No dialog.
bool
ui::dialogs::MessageBox::doYesNoDialog(afl::string::Translator& tx)
{
    // ex UIMessageBox::doYesNoDialog
    util::KeyString y(tx("Yes")), n(tx("No"));
    addButton(1, y.getString(), y.getKey());
    addButton(0, n.getString(), n.getKey());
    addKey(1, ' ');
    return run() != 0;

}

// Build and operate a simple confirmation dialog (just an OK button).
void
ui::dialogs::MessageBox::doOkDialog(afl::string::Translator& tx)
{
    // ex UIMessageBox::doOkDialog
    addButton(1, tx("OK"), ' ');
    run();
}

/** Initialize.
    Build the initial prototype widget. */
void
ui::dialogs::MessageBox::init(const util::rich::Text& text)
{
    // ex UIMessageBox::init
    // FIXME:     setLayout(h.add(new UIVBoxLayout(10)));

    // Make text a little bigger
    util::rich::Text bigText(text);
    bigText.withStyle(util::rich::StyleAttribute::Big);

    // Estimate size
    // FIXME: magic numbers
    ui::rich::Document tmp(m_root.provider());
    tmp.setPageWidth(440);
    tmp.add(bigText);
    tmp.finish();
    int width = tmp.getDocumentWidth();
    if (width < 200) {
        width = 200;
    }

    // Build widgets
    add(m_deleter.addNew(new ui::rich::StaticText(bigText, width, m_root.provider())));
    add(m_buttonGroup);
    add(m_keyDispatcher);

    // Prepare widgets
    m_buttonGroup.add(m_deleter.addNew(new Spacer()));
}

/** Check key/command assignment.
    This is used to figure out whether the user mapped the Return/Escape keys,
    and what the first/last buttons are. */
void
ui::dialogs::MessageBox::checkKey(int id, util::Key_t key, bool isButton)
{
    if (isButton) {
        if (!m_flags.contains(HaveFirst)) {
            m_firstCommand = id;
            m_flags += HaveFirst;
        }
        m_lastCommand = id;
    }
    if (key == util::Key_Escape) {
        m_flags += HaveEscape;
    }
    if (key == util::Key_Return) {
        m_flags += HaveReturn;
    }
}
