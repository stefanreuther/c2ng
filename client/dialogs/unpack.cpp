/**
  *  \file client/dialogs/unpack.cpp
  *  \brief Unpack dialog
  */

#include "client/dialogs/unpack.hpp"
#include "afl/base/deleter.hpp"
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
#include "client/dialogs/simpleconsole.hpp"
#include "afl/base/signalconnection.hpp"

using client::widgets::PlayerSetSelector;
using ui::rich::StaticText;
using ui::widgets::FocusIterator;
using ui::widgets::KeyDispatcher;
using ui::widgets::StandardDialogButtons;
using util::rich::Parser;

namespace {
    /*
     *  Unpack Dialog
     */
    class Dialog {
     public:
        Dialog(const game::proxy::MaintenanceProxy::UnpackStatus& status, ui::Root& root, afl::string::Translator& tx)
            : m_root(root),
              m_translator(tx),
              m_playerSelector(root, status.playerNames, status.allPlayers, tx),
              m_applyTurnFlag(1),
              m_applyTurnCheckbox(root, util::KeyMod_Alt + 'a', tx("Apply turn files?"), m_applyTurnFlag),
              m_loop(root)
            {
                // ex WUnpackWindow::WUnpackWindow
                m_applyTurnCheckbox.addDefaultImages();
                m_playerSelector.setSelectedPlayers(status.selectedPlayers);
                m_playerSelector.setSelectablePlayers(status.availablePlayers);
                m_playerSelector.setCurrentItem(0);
                for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
                    if (status.availablePlayers.contains(i)) {
                        m_playerSelector.setItemInfo(m_playerSelector.findItem(i), status.turnFilePlayers.contains(i) ? "RST + TRN" : "RST");
                    }
                }
            }

        bool run(ui::Widget* pHelp);

        void onOK();

        void startUnpack(game::proxy::MaintenanceProxy& proxy)
            { proxy.startUnpack(m_playerSelector.getSelectedPlayers(), m_applyTurnFlag.get() != 0); }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        PlayerSetSelector m_playerSelector;
        afl::base::Observable<int> m_applyTurnFlag;
        ui::widgets::Checkbox m_applyTurnCheckbox;
        ui::EventLoop m_loop;
    };
}

bool
Dialog::run(ui::Widget* pHelp)
{
    // ex WUnpackWindow::init
    afl::base::Deleter del;

    // VBox
    //   "Choose files to unpack"
    //   HBox
    //     PlayerSetSelector
    //     VBox StaticText, CheckboxText, Spacer
    //   StandardDialogButtons
    ui::Window& win = del.addNew(new ui::Window(m_translator("Unpack"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(del.addNew(new ui::widgets::StaticText(m_translator("Choose files to unpack:"), util::SkinColor::Static, "+", m_root.provider())));

    ui::Group& g12 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    g12.add(del.addNew(new StaticText(Parser::parseXml(m_translator("The <em>Unpack</em> function will unpack result files into "
                                                                    "individual files. After unpacking, you can play your "
                                                                    "turn using all VGAP utilities of your choice. When "
                                                                    "done with the turn, use the <em>Maketurn</em> function to "
                                                                    "generate TRN files to send to the host.\n\n"
                                                                    "Note that PCC2 doesn't require you to unpack a result "
                                                                    "file prior to playing, but most other utilities do.")),
                                      300, m_root.provider())));
    g12.add(m_applyTurnCheckbox);
    g12.add(del.addNew(new StaticText(Parser::parseXml(m_translator("With this option selected, PCC2 will return you to the status of the last Maketurn. "
                                                                    "Otherwise, unpacking results will bring you to the beginning of the current turn.\n\n")),
                                      300, m_root.provider())));
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
    it.add(m_applyTurnCheckbox);
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
    // WUnpackWindow::onOK()
    if (m_playerSelector.getSelectedPlayers().empty()) {
        // Nothing selected - cancel
        m_loop.stop(0);
    } else {
        // OK
        m_loop.stop(1);
    }
}

bool
client::dialogs::doUnpackDialog(game::proxy::MaintenanceProxy& proxy, ui::Widget* pHelp, ui::Root& root, afl::string::Translator& tx)
{
    // ex doUnpack
    // Retrieve initial status
    Downlink link(root, tx);
    game::proxy::MaintenanceProxy::UnpackStatus st = proxy.prepareUnpack(link);
    if (!st.valid || st.availablePlayers.empty()) {
        ui::dialogs::MessageBox(tx("This directory contains no files to unpack."), tx("Unpack"), root).doOkDialog(tx);
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
    dlg.startUnpack(proxy);
    console.run(tx("Unpack"));
    return true;
}
