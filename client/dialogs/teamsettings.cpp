/**
  *  \file client/dialogs/teamsettings.cpp
  *  \brief Team editor dialog
  */

#include "client/dialogs/teamsettings.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/playerlist.hpp"
#include "game/limits.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/teamproxy.hpp"
#include "ui/group.hpp"
#include "ui/icons/colortile.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/icongrid.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using afl::string::Format;
using client::widgets::PlayerList;
using game::PlayerArray;
using game::PlayerSet_t;
using game::TeamSettings;
using ui::widgets::Button;
using ui::widgets::InputLine;
using ui::widgets::KeyDispatcher;
using ui::Group;

namespace {
    class TeamSettingsDialog {
     public:
        TeamSettingsDialog(ui::Root& root, TeamSettings& settings, PlayerSet_t allPlayers, afl::string::Translator& tx);

        bool run(util::RequestSender<game::Session> gameSender, const PlayerArray<String_t>& playerNames);

     private:
        ui::Root& m_root;
        TeamSettings& m_settings;
        afl::string::Translator& m_translator;
        PlayerSet_t m_allPlayers;
        ui::icons::ColorTile m_redTile;
        ui::icons::ColorTile m_greenTile;
        ui::widgets::IconGrid m_grid;
        ui::widgets::StaticText m_teamName;

        void onSetTeam();
        void onItemSelected();
        void onFocusTeam();
        void onEditName();
        void onTeamChange();

        static gfx::Point getCellSize(ui::Root& root);
    };
}


TeamSettingsDialog::TeamSettingsDialog(ui::Root& root, TeamSettings& settings, PlayerSet_t allPlayers, afl::string::Translator& tx)
    : m_root(root),
      m_settings(settings),
      m_translator(tx),
      m_allPlayers(allPlayers),
      m_redTile(root, getCellSize(root), ui::Color_Fire + 7),
      m_greenTile(root, getCellSize(root), ui::Color_GreenScale + 11),
      m_grid(m_root.engine(), getCellSize(root), int(allPlayers.size()), int(allPlayers.size())),
      m_teamName("", util::SkinColor::Static, gfx::FontRequest(), root.provider(), gfx::LeftAlign)
{
    m_grid.sig_doubleClick.add(this, &TeamSettingsDialog::onSetTeam);
    m_grid.sig_itemSelected.add(this, &TeamSettingsDialog::onItemSelected);
    m_teamName.setIsFlexible(true);
}

bool
TeamSettingsDialog::run(util::RequestSender<game::Session> gameSender, const PlayerArray<String_t>& playerNames)
{
    afl::base::SignalConnection conn_teamChange(m_settings.sig_teamChange
                                                .add(this, &TeamSettingsDialog::onTeamChange));

    // ex WTeamEditWindow::buildDialog()
    // Window[UIVBoxLayout]
    //   Group[Grid]
    //     Spacer     PlayerList
    //     PlayerList IconGrid
    //   Group[HBox]
    //     StaticText Button
    //   Group[HBox]
    //     Checkbox "Auto Sync"
    //   StandardDialogButtons
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Teams"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // FIXME: auto-sync
    //     pAutoSync = &holder.add(new UICheckbox(1001, m_translator("Auto-sync with alliances"), 'a', 2));
    //     pAutoSync->setValue(getUserPreferences().TeamAutoSync());

    PlayerList& p1 = del.addNew(new PlayerList(m_root, PlayerList::HorizontalLayout, PlayerList::ShowLetters, PlayerList::SameColors, 0, m_allPlayers));
    PlayerList& p2 = del.addNew(new PlayerList(m_root, PlayerList::VerticalLayout,   PlayerList::ShowNames,   PlayerList::SameColors, 0, m_allPlayers));
    p1.setNames(playerNames);
    p2.setNames(playerNames);

    Group& g1 = del.addNew(new Group(del.addNew(new ui::layout::Grid(2))));
    g1.add(del.addNew(new ui::Spacer()));
    g1.add(p1);
    g1.add(p2);
    g1.add(m_grid);
    win.add(g1);

    Button& btnName = del.addNew(new Button("N", 'n', m_root));
    Group& g2 = del.addNew(new Group(ui::layout::HBox::instance5));
    g2.add(m_teamName);
    g2.add(btnName);
    btnName.sig_fire.add(this, &TeamSettingsDialog::onEditName);
    win.add(g2);

    //     UIGroup& g3 = holder.add(new UIGroup(UIHBoxLayout::instance5));
    //     g3.add(*pAutoSync);
    //     add(g3);

    client::widgets::HelpWidget help(m_root, m_translator, gameSender, "pcc2:teams");
    ui::EventLoop loop(m_root);
    ui::widgets::StandardDialogButtons& g4 = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    g4.addStop(loop);
    g4.addHelp(help);

    //     add(holder.add(new UIFocusIterator(1000, 1001, UIFocusIterator::fi_Tab)));

    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    win.add(g4);
    win.add(help);

    // ex WTeamGrid::handleEvent, team.pas:CTeamPanel.Handle (part)
    KeyDispatcher& disp = del.addNew(new KeyDispatcher());
    disp.add(' ',                                 this, &TeamSettingsDialog::onSetTeam);
    disp.add(util::Key_Left  + util::KeyMod_Ctrl, this, &TeamSettingsDialog::onFocusTeam);
    disp.add(util::Key_Right + util::KeyMod_Ctrl, this, &TeamSettingsDialog::onFocusTeam);
    win.add(disp);

    win.pack();
    onTeamChange();
    m_grid.setCurrentItem(m_settings.getPlayerTeam(m_settings.getViewpointPlayer()) - 1,
                          m_settings.getViewpointPlayer() - 1);
    m_grid.requestFocus();
    onItemSelected();

    m_root.centerWidget(win);
    m_root.add(win);
    return loop.run() != 0;
}

void
TeamSettingsDialog::onSetTeam()
{
    // Space/double-click: place team marker here
    m_settings.setPlayerTeam(m_grid.getCurrentLine()+1, m_grid.getCurrentColumn()+1);
}

void
TeamSettingsDialog::onItemSelected()
{
    // ex WTeamEditWindow::onCursorMove, WTeamName::drawContent, team.pas:CTeamPanel.ShowName
    const int teamNr = m_grid.getCurrentColumn() + 1;
    if (m_settings.isNamedTeam(teamNr)) {
        m_teamName.setText(Format(m_translator("Team %d: %s"), teamNr, m_settings.getTeamName(teamNr, m_translator)));
    } else {
        m_teamName.setText(Format(m_translator("Team %d"), teamNr));
    }
}

void
TeamSettingsDialog::onFocusTeam()
{
    // Ctrl+Left/Right: place cursor on this line's team marker
    const int playerNr = m_grid.getCurrentLine() + 1;
    const int teamNr   = m_settings.getPlayerTeam(playerNr);
    m_grid.setCurrentItem(teamNr - 1, playerNr - 1);
}

void
TeamSettingsDialog::onEditName()
{
    // ex WTeamEditWindow::onEditName, team.pas:EditTeamName
    // Prepare team name
    const int teamNr = m_grid.getCurrentColumn() + 1;

    InputLine input(200, 30, m_root);
    if (m_settings.isNamedTeam(teamNr)) {
        input.setText(m_settings.getTeamName(teamNr, m_translator));
    }
    input.setFlag(InputLine::GameChars, true);

    // Edit it
    if (input.doStandardDialog(Format(m_translator("Team %d"), teamNr), m_translator("Team Name:"), m_translator)) {
        m_settings.setTeamName(teamNr, input.getText());
    }
}

void
TeamSettingsDialog::onTeamChange()
{
    // WTeamGrid::drawContent, team.pas:CTeamPanel.Draw
    // Render entire grid
    const int myTeam = m_settings.getPlayerTeam(m_settings.getViewpointPlayer());
    const int size = int(m_allPlayers.size());
    for (int player = 1; player <= size; ++player) {
        const int playerTeam = m_settings.getPlayerTeam(player);
        for (int team = 1; team <= size; ++team) {
            m_grid.setIcon(team-1, player-1, (team == playerTeam ? (team == myTeam ? &m_greenTile : &m_redTile) : 0));
        }
    }

    // Update name display
    onItemSelected();
}

gfx::Point
TeamSettingsDialog::getCellSize(ui::Root& root)
{
    int dim = root.provider().getFont(gfx::FontRequest())->getLineHeight() - 1;
    return gfx::Point(dim, dim);
}



/*
 *  Entry Point
 */


void
client::dialogs::editTeams(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
{
    // Proxies
    game::proxy::TeamProxy teamProxy(gameSender);
    game::proxy::PlayerProxy playerProxy(gameSender);

    // Load initial state
    Downlink link(root, tx);
    game::TeamSettings settings;
    teamProxy.init(link, settings);

    PlayerArray<String_t> playerNames = playerProxy.getPlayerNames(link, game::Player::ShortName);
    PlayerSet_t allPlayers = playerProxy.getAllPlayers(link);

    // Normalize allPlayers
    // The idea is to have a 1:1 mapping between rows/columns and player numbers.
    // Not having to expect holes simplifies the code a lot; allPlayers.size() will be the highest player number.
    // Even if a game has no "player 3", there can be a "team 3".
    for (int i = game::MAX_PLAYERS; i > 0; --i) {
        if (allPlayers.contains(i)) {
            allPlayers = PlayerSet_t::allUpTo(i) - 0;
            break;
        }
    }

    // Dialog
    TeamSettingsDialog dlg(root, settings, allPlayers, tx);
    if (dlg.run(gameSender, playerNames)) {
        teamProxy.commit(settings);
    }
}
