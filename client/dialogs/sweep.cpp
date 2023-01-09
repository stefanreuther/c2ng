/**
  *  \file client/dialogs/sweep.cpp
  *  \brief Directory Cleanup (Sweep) Dialog
  */

#include "client/dialogs/sweep.hpp"
#include "afl/base/deleter.hpp"
#include "client/dialogs/simpleconsole.hpp"
#include "client/downlink.hpp"
#include "client/widgets/playersetselector.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/checkbox.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/rich/parser.hpp"

using client::widgets::PlayerSetSelector;
using ui::rich::StaticText;
using ui::widgets::FocusIterator;
using ui::widgets::KeyDispatcher;
using ui::widgets::StandardDialogButtons;
using util::rich::Parser;

namespace {
    /*
     *  Sweep Dialog
     */
    class Dialog {
     public:
        Dialog(const game::proxy::MaintenanceProxy::SweepStatus& status, ui::Root& root, afl::string::Translator& tx)
            : m_root(root),
              m_translator(tx),
              m_playerSelector(root, status.playerNames, status.allPlayers, tx),
              m_eraseDatabaseFlag(0),
              m_eraseDatabaseCheckbox(root, util::KeyMod_Alt + 'd', tx("Delete database files?"), m_eraseDatabaseFlag),
              m_loop(root)
            {
                // ex WSweepWindow::WSweepWindow
                m_eraseDatabaseCheckbox.addDefaultImages();
                m_playerSelector.setSelectedPlayers(status.selectedPlayers);
            }

        bool run(ui::Widget* pHelp);

        void onOK();

        void startSweep(game::proxy::MaintenanceProxy& proxy)
            { proxy.startSweep(m_playerSelector.getSelectedPlayers(), m_eraseDatabaseFlag.get() != 0); }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        PlayerSetSelector m_playerSelector;
        afl::base::Observable<int> m_eraseDatabaseFlag;
        ui::widgets::Checkbox m_eraseDatabaseCheckbox;
        ui::EventLoop m_loop;
    };
}

bool
Dialog::run(ui::Widget* pHelp)
{
    // ex WSweepWindow::init
    afl::base::Deleter del;

    // VBox
    //   "Choose player slots to clean"
    //   HBox
    //     PlayerSetSelector
    //     VBox StaticText, CheckboxText, Spacer
    //   StandardDialogButtons
    ui::Window& win = del.addNew(new ui::Window(m_translator("Clean Up (Sweep)"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(del.addNew(new ui::widgets::StaticText(m_translator("Choose player slots to clean:"), util::SkinColor::Static, "+", m_root.provider())));

    ui::Group& g12 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    g12.add(del.addNew(new StaticText(Parser::parseXml(m_translator("The <em>Clean Up (Sweep)</em> function will remove player data files. "
                                                                    "By default, it will delete only files that can be "
                                                                    "restored by re-unpacking a result file.\n\n"
                                                                    "With the <em>Database</em> option, this function will also delete "
                                                                    "the history database and configuration files.")),
                                      300, m_root.provider())));
    g12.add(m_eraseDatabaseCheckbox);
    g12.add(del.addNew(new ui::Spacer()));

    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g1.add(del.addNew(new ui::widgets::ScrollbarContainer(m_playerSelector, m_root)));
    g1.add(g12);
    win.add(g1);

    // Buttons
    StandardDialogButtons& btn = del.addNew(new StandardDialogButtons(m_root, m_translator));
    if (pHelp != 0) {
        btn.addHelp(*pHelp);
        win.add(*pHelp);
    }
    btn.ok().sig_fire.add(this, &Dialog::onOK);
    btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    win.add(btn);

    // Focus
    FocusIterator& it = del.addNew(new FocusIterator(FocusIterator::Horizontal | FocusIterator::Tab));
    it.add(m_playerSelector);
    it.add(m_eraseDatabaseCheckbox);
    win.add(it);

    // Keys
    KeyDispatcher& disp = del.addNew(new KeyDispatcher());
    disp.add('*', &m_playerSelector, &PlayerSetSelector::toggleAll);
    win.add(disp);

    // Do it
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    return m_loop.run() != 0;
}

void
Dialog::onOK()
{
    // WSweepWindow::onEnter
    if (m_playerSelector.getSelectedPlayers().empty()) {
        // Nothing selected - cancel
        m_loop.stop(0);
    } else if (!ui::dialogs::MessageBox(m_translator("Delete these players' files?"), m_translator("Clean Up (Sweep)"), m_root).doYesNoDialog(m_translator)) {
        // Canceled
        m_loop.stop(0);
    } else {
        // OK
        m_loop.stop(1);
    }
}


bool
client::dialogs::doSweepDialog(game::proxy::MaintenanceProxy& proxy, ui::Widget* pHelp, ui::Root& root, afl::string::Translator& tx)
{
    // ex doSweep
    // Retrieve initial status
    Downlink link(root, tx);
    game::proxy::MaintenanceProxy::SweepStatus st = proxy.prepareSweep(link);
    if (!st.valid) {
        return false;
    }

    // Main dialog
    Dialog dlg(st, root, tx);
    if (!dlg.run(pHelp)) {
        return false;
    }

    // Run it
    SimpleConsole console(root, tx);
    afl::base::SignalConnection conn_message(proxy.sig_message.add(&console, &SimpleConsole::addMessage));
    afl::base::SignalConnection conn_actionComplete(proxy.sig_actionComplete.add(&console, &SimpleConsole::enableClose));
    dlg.startSweep(proxy);
    console.run(tx("Clean Up (Sweep)"));

    return true;
}
