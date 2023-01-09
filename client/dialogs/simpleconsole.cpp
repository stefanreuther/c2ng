/**
  *  \file client/dialogs/simpleconsole.cpp
  *  \brief Class client::dialogs::SimpleConsole
  */

#include "client/dialogs/simpleconsole.hpp"
#include "afl/base/deleter.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/window.hpp"

using ui::widgets::KeyDispatcher;

client::dialogs::SimpleConsole::SimpleConsole(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_consoleView(root.provider(), gfx::Point(35, 18)),
      m_consoleController(m_consoleView),
      m_closeButton(tx("Close"), ' ', root),
      m_loop(root),
      m_allowClose(false)
{
    updateClose();
    m_closeButton.sig_fire.add(this, &SimpleConsole::onClose);
}

void
client::dialogs::SimpleConsole::addMessage(String_t str)
{
    gfx::HorizontalAlignment align = str.find('\t') != String_t::npos ? gfx::CenterAlign : gfx::LeftAlign;
    m_consoleController.addLine(str, align, 0, util::SkinColor::Static);
}

void
client::dialogs::SimpleConsole::enableClose()
{
    m_allowClose = true;
    updateClose();
}

void
client::dialogs::SimpleConsole::run(String_t title)
{
    // ex WConsoleLogWindow::init, WConsoleLogWindow::open, WConsoleLogWindow::waitForConfirmation (sort-of)
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(m_consoleController);
    win.add(m_consoleView);

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(del.addNew(new ui::Spacer()));
    g.add(m_closeButton);
    win.add(g);

    KeyDispatcher& disp = del.addNew(new KeyDispatcher());
    disp.add(util::Key_Return, this, &SimpleConsole::onClose);
    disp.add(util::Key_Escape, this, &SimpleConsole::onClose);
    win.add(disp);
    // FIXME: deal with Quit

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

void
client::dialogs::SimpleConsole::onClose()
{
    if (m_allowClose) {
        m_loop.stop(0);
    }
}

void
client::dialogs::SimpleConsole::updateClose()
{
    m_closeButton.setState(ui::Widget::DisabledState, !m_allowClose);
}
