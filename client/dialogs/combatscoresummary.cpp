/**
  *  \file client/dialogs/combatscoresummary.cpp
  *  \brief Combat Score Summary dialog
  */

#include "client/dialogs/combatscoresummary.hpp"
#include "afl/bits/smallset.hpp"
#include "client/downlink.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/vcroverviewproxy.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "game/proxy/configurationproxy.hpp"

using game::vcr::Score;

namespace {
    enum Column {
        ExperienceExists,
        ExperienceRange,
        BuildPointsExist,
        BuildPointRange,
        TonsExist,
        TonsRange
    };
    typedef afl::bits::SmallSet<Column> Columns_t;

    void checkRange(Columns_t& columns, const Score::Range_t range, int scale, Column exists, Column hasRange)
    {
        int32_t min = range.min() / scale;
        int32_t max = range.max() / scale;
        if (max > 0) {
            columns += exists;
            if (max > min) {
                columns += hasRange;
            }
        }
    }

    void checkScores(Columns_t& columns, const Score& score)
    {
        checkRange(columns, score.getExperience(),       1,    ExperienceExists, ExperienceRange);
        checkRange(columns, score.getBuildMillipoints(), 1000, BuildPointsExist, BuildPointRange);
        checkRange(columns, score.getTonsDestroyed(),    1,    TonsExist,        TonsRange);
    }

    void renderHeading(ui::widgets::SimpleTable& tab, size_t& col, size_t row, String_t label, bool exists, bool hasRange)
    {
        if (exists) {
            tab.setColumnPadding(col-1, 5);
            tab.cell(col, row).setText(label).setTextAlign(gfx::CenterAlign, gfx::TopAlign).setUnderline(true).setColor(ui::Color_Black).setFont("b");
            if (hasRange) {
                // Two columns
                tab.cell(col, row).setExtraColumns(1);
                col += 2;
            } else {
                // One column
                ++col;
            }
        }
    }

    void renderRange(ui::widgets::SimpleTable& tab, size_t& col, size_t row, const Score::Range_t& range, int scale, bool exists, bool hasRange, const util::NumberFormatter& fmt)
    {
        if (exists) {
            int32_t min = range.min() / scale;
            int32_t max = range.max() / scale;
            if (hasRange) {
                if (max == 0) {
                    tab.cell(col, row).setText("- ").setTextAlign(gfx::RightAlign, gfx::TopAlign).setColor(ui::Color_Dark);
                } else {
                    tab.cell(col, row).setText(fmt.formatNumber(min) + " ... ").setTextAlign(gfx::RightAlign, gfx::TopAlign).setColor(ui::Color_Black);
                    tab.cell(col+1, row).setText(fmt.formatNumber(max)).setColor(ui::Color_Black);
                }
                col += 2;
            } else {
                if (max == 0) {
                    tab.cell(col, row).setText("-").setTextAlign(gfx::RightAlign, gfx::TopAlign).setColor(ui::Color_Dark);
                } else {
                    tab.cell(col, row).setText(fmt.formatNumber(max)).setTextAlign(gfx::RightAlign, gfx::TopAlign).setColor(ui::Color_Black);
                }
                ++col;
            }
        }
    }

    void renderScores(ui::widgets::SimpleTable& tab, size_t row, String_t playerName, const Score& score, Columns_t columns, const util::NumberFormatter& fmt)
    {
        tab.cell(0, row).setText(playerName).setColor(ui::Color_Black);

        size_t c = 1;
        renderRange(tab, c, row, score.getExperience(),          1, columns.contains(ExperienceExists), columns.contains(ExperienceRange), fmt);
        renderRange(tab, c, row, score.getBuildMillipoints(), 1000, columns.contains(BuildPointsExist), columns.contains(BuildPointRange), fmt);
        renderRange(tab, c, row, score.getTonsDestroyed(),       1, columns.contains(TonsExist),        columns.contains(TonsRange),       fmt);
    }
}

void
client::dialogs::showCombatScoreSummary(ui::Root& root,
                                        afl::string::Translator& tx,
                                        util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender,
                                        util::RequestSender<game::Session> gameSender)
{
    // ex showCombatScores
    game::vcr::Overview::ScoreSummary scores;
    Downlink link(root, tx);
    game::proxy::VcrOverviewProxy(vcrSender).buildScoreSummary(link, scores);
    if (scores.players.empty()) {
        return;
    }

    // Find columns
    Columns_t columns;
    for (int i = 1; i < game::MAX_PLAYERS; ++i) {
        if (scores.players.contains(i)) {
            checkScores(columns, scores.scores.get(i));
        }
    }
    if (columns.empty()) {
        // This should never happen as we always compute tons. Therefore,
        // no need to do any fancy error messaging.
        return;
    }

    // Get remaining environment
    game::PlayerArray<String_t> names = game::proxy::PlayerProxy(gameSender).getPlayerNames(link, game::Player::LongName);
    util::NumberFormatter fmt = game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link);

    // Build table
    ui::widgets::SimpleTable tab(root, columns.size() + 1, scores.players.size() + 1);

    // Headings
    size_t c = 1;
    size_t r = 0;
    renderHeading(tab, c, r, tx("Experience"),   columns.contains(ExperienceExists), columns.contains(ExperienceRange));
    renderHeading(tab, c, r, tx("Build Points"), columns.contains(BuildPointsExist), columns.contains(BuildPointRange));
    renderHeading(tab, c, r, tx("Tons sunk"),    columns.contains(TonsExist),        columns.contains(TonsRange));
    ++r;

    // Players
    for (int i = 1; i < game::MAX_PLAYERS; ++i) {
        if (scores.players.contains(i)) {
            renderScores(tab, r, names.get(i), scores.scores.get(i), columns, fmt);
            ++r;
        }
    }

    // Display it
    afl::base::Deleter del;
    ui::Window win(tx("Combat Score Overview"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
    win.add(tab);

    if (scores.numBattles != 1) {
        win.add(del.addNew(new ui::widgets::StaticText(afl::string::Format(tx("This overview covers all %d fights."), scores.numBattles),
                                                       util::SkinColor::Static, gfx::FontRequest(), root.provider())));
    }

    ui::Group& group = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnOK = del.addNew(new ui::widgets::Button(tx("OK"), util::Key_Return, root));
    group.add(del.addNew(new ui::Spacer()));
    group.add(btnOK);
    group.add(del.addNew(new ui::Spacer()));
    win.add(group);

    ui::EventLoop loop(root);
    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    disp.addNewClosure(' ', loop.makeStop(0));
    disp.addNewClosure(util::Key_Escape, loop.makeStop(0));
    btnOK.sig_fire.addNewClosure(loop.makeStop(0));

    win.add(disp);
    win.add(del.addNew(new ui::widgets::Quit(root, loop)));

    win.pack();
    root.centerWidget(win);
    root.add(win);
    loop.run();
}
