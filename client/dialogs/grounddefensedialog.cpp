/**
  *  \file client/dialogs/grounddefensedialog.cpp
  *  \brief Ground Defense Dialog
  */

#include "client/dialogs/grounddefensedialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/widgets/simpletable.hpp"
#include "util/skincolor.hpp"

using util::SkinColor;

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
client::dialogs::doGroundDefenseDialog(ui::Root& root, const game::map::GroundDefenseInfo& info, util::NumberFormatter fmt, afl::string::Translator& tx)
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
    tab.all().setColor(SkinColor::Static);
    tab.column(1).setTextAlign(gfx::RightAlign, gfx::TopAlign);
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
                tab.cell(1, row).setText(afl::string::Format(tx("(defense) %d"), fmt.formatNumber(info.strength.get(i))));
                tab.row(row).setColor(SkinColor::Green);
            } else {
                tab.cell(1, row).setText(fmt.formatNumber(info.strength.get(i)));
            }
        }
    }

    ui::dialogs::MessageBox(tab, tx("Ground Combat"), root)
        .doOkDialog(tx);
}
