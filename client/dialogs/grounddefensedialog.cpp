/**
  *  \file client/dialogs/grounddefensedialog.cpp
  *  \brief Ground Defense Dialog
  */

#include "client/dialogs/grounddefensedialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"
#include "ui/draw.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/window.hpp"

namespace {
    /* Check whether to display a player row.
       We show all rows that we have information (=a player name) for.
       Defender is also shown if it has no name (misconfiguration). */
    bool isPlayerVisible(const game::map::GroundDefenseInfo& info, int player)
    {
        return player == info.defender
            || !info.name.get(player).empty();
    }
}

void
client::dialogs::doGroundDefenseDialog(ui::Root& root, const game::map::GroundDefenseInfo& info, afl::string::Translator& tx)
{
    // ex doGroundCombatPrediction
    // ex envscan.pas:CGroundCombatWindow.DrawInterior, ShowGroundCombatChances

    // Count number of lines
    size_t numRows = 0;
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (isPlayerVisible(info, i)) {
            ++numRows;
        }
    }

    // Build table
    afl::base::Deleter del;
    ui::widgets::SimpleTable& tab = del.addNew(new ui::widgets::SimpleTable(root, 2, numRows));
    tab.all().setColor(ui::Color_Black);
    tab.column(1).setTextAlign(2, 0);
    tab.row(0).setUnderline(true);
    tab.cell(0, 0).setText(tx("Attacker"));
    tab.cell(1, 0).setText(tx("Clans needed to win"));
    tab.setColumnPadding(0, 5);

    size_t row = 0;
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (isPlayerVisible(info, i)) {
            ++row;
            tab.cell(0, row).setText(info.name.get(i));
            if (i == info.defender) {
                // FIXME: use NumberFormatter
                tab.cell(1, row).setText(afl::string::Format(tx("(defense) %d"), info.strength.get(i)));
                tab.row(row).setColor(ui::Color_GreenBlack);
            } else {
                // FIXME: use NumberFormatter
                tab.cell(1, row).setText(afl::string::Format("%d", info.strength.get(i)));
            }
        }
    }

    // Loop
    ui::EventLoop loop(root);

    // Window
    ui::Window& win = del.addNew(new ui::Window(tx("Ground Combat"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(tab);

    // - buttons
    ui::widgets::Button& btnOK = del.addNew(new ui::widgets::Button(tx("OK"), util::Key_Return, root));
    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnOK);
    g.add(del.addNew(new ui::Spacer()));
    win.add(g);

    // - keys
    ui::widgets::KeyDispatcher& keys = del.addNew(new ui::widgets::KeyDispatcher());
    keys.addNewClosure(' ', loop.makeStop(0));
    keys.addNewClosure(util::Key_Escape, loop.makeStop(0));
    btnOK.sig_fire.addNewClosure(loop.makeStop(0));
    win.add(keys);

    // Do it
    win.pack();
    root.centerWidget(win);
    root.add(win);
    loop.run();
}
