/**
  *  \file client/dialogs/simulationfleetcost.cpp
  *  \brief Simulation Fleet Cost Dialog
  */

#include "client/dialogs/simulationfleetcost.hpp"
#include "afl/base/deleter.hpp"
#include "client/dialogs/simulationfleetcostoptions.hpp"
#include "client/downlink.hpp"
#include "client/widgets/costsummarylist.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/fleetcostproxy.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/teamproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

namespace {
    int getNextPlayer(game::PlayerSet_t set, int player, int delta)
    {
        // ex fleetcost.cc:getNextPlayer
        for (int i = 0; i < game::MAX_PLAYERS; ++i) {
            player += delta;
            if (player > game::MAX_PLAYERS) {
                player = 1;
            }
            if (player <= 0) {
                player = game::MAX_PLAYERS;
            }
            if (set.contains(player)) {
                break;
            }
        }
        return player;
    }


    class FleetCostDialog {
     public:
        FleetCostDialog(ui::Root& root, game::proxy::SimulationSetupProxy& setupProxy, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

        bool init();
        void run();

     private:
        ui::Root& m_root;
        game::proxy::FleetCostProxy m_costProxy;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;

        // Widgets
        ui::widgets::StaticText m_label;
        client::widgets::CostSummaryList m_costSummary;

        // Fixed status
        game::PlayerSet_t m_involvedPlayers;
        game::PlayerSet_t m_involvedTeams;
        game::PlayerArray<String_t> m_playerNames;
        game::TeamSettings m_teamSettings;
        bool m_teamsAvailable;
        bool m_teamsActive;

        // Variable status
        int m_currentPlayer;
        int m_currentTeam;

        void render(client::Downlink& link);
        void onEditOptions();
        void onNext();
        void onPrevious();
        void browse(int delta);
    };
}

FleetCostDialog::FleetCostDialog(ui::Root& root,
                                 game::proxy::SimulationSetupProxy& setupProxy,
                                 util::RequestSender<game::Session> gameSender,
                                 afl::string::Translator& tx)
    : m_root(root),
      m_costProxy(setupProxy),
      m_gameSender(gameSender),
      m_translator(tx),
      m_label("", util::SkinColor::Static, "", root.provider()),
      m_costSummary(20, true, client::widgets::CostSummaryList::TotalsFooter, root, tx),
      m_involvedPlayers(),
      m_involvedTeams(),
      m_playerNames(),
      m_teamSettings(),
      m_teamsAvailable(false),
      m_teamsActive(false),
      m_currentPlayer(),
      m_currentTeam()
{ }

bool
FleetCostDialog::init()
{
    // ex WFleetCostDialog::init (part)
    client::Downlink link(m_root, m_translator);

    // Player list
    m_involvedPlayers = m_costProxy.getInvolvedPlayers(link);
    if (m_involvedPlayers.empty()) {
        return false;
    }

    // Team list
    m_involvedTeams = m_costProxy.getInvolvedTeams(link);
    game::proxy::TeamProxy(m_gameSender).init(link, m_teamSettings);
    m_teamsAvailable = !m_involvedTeams.empty() && m_teamSettings.hasAnyTeams();

    // Player names
    m_playerNames = game::proxy::PlayerProxy(m_gameSender).getPlayerNames(link, game::Player::ShortName);

    // Current player
    m_currentPlayer = m_teamSettings.getViewpointPlayer();
    if (m_currentPlayer == 0 || !m_involvedPlayers.contains(m_currentPlayer)) {
        m_currentPlayer = getNextPlayer(m_involvedPlayers, 0, 1);
    }
    m_currentTeam = m_teamSettings.getPlayerTeam(m_currentPlayer);

    // Widgets
    m_label.setIsFlexible(true);

    // Initial content
    render(link);
    return true;
}

void
FleetCostDialog::run()
{
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(m_translator("Fleet Cost Comparison"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(m_costSummary);

    ui::Group& cycleGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnPrev = del.addNew(new ui::widgets::Button("<", util::Key_Tab + util::KeyMod_Shift, m_root));
    ui::widgets::Button& btnNext = del.addNew(new ui::widgets::Button(">", util::Key_Tab, m_root));
    cycleGroup.add(btnPrev);
    cycleGroup.add(m_label);
    cycleGroup.add(btnNext);
    win.add(cycleGroup);

    ui::Group& buttonGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnClose   = del.addNew(new ui::widgets::Button(m_translator("Close"),       util::Key_Escape, m_root));
    ui::widgets::Button& btnOptions = del.addNew(new ui::widgets::Button(m_translator("O - Options"), 'o',              m_root));
    ui::widgets::Button& btnExport  = del.addNew(new ui::widgets::Button(m_translator("E - Export"), 'e', m_root));
    ui::widgets::Button& btnHelp    = del.addNew(new ui::widgets::Button(m_translator("Help"),        'h',              m_root));
    buttonGroup.add(btnClose);
    buttonGroup.add(btnOptions);
    buttonGroup.add(btnExport);
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(btnHelp);
    win.add(buttonGroup);

    ui::EventLoop loop(m_root);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:fleetcost"));
    win.add(help);

    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    win.add(disp);

    btnClose.sig_fire.addNewClosure(loop.makeStop(0));
    btnPrev.dispatchKeyTo(disp);
    btnNext.dispatchKeyTo(disp);
    btnOptions.dispatchKeyTo(disp);
    btnExport.dispatchKeyTo(disp);
    btnHelp.dispatchKeyTo(help);

    disp.addNewClosure(util::Key_Return, loop.makeStop(0));
    disp.addNewClosure('e', m_costSummary.makeExporter(m_gameSender));
    disp.add(util::Key_Left,                     this, &FleetCostDialog::onPrevious);
    disp.add(util::Key_Tab | util::KeyMod_Shift, this, &FleetCostDialog::onPrevious);
    disp.add(util::Key_Right,                    this, &FleetCostDialog::onNext);
    disp.add(util::Key_Tab,                      this, &FleetCostDialog::onNext);
    disp.add('o',                                this, &FleetCostDialog::onEditOptions);
    disp.add('o' | util::KeyMod_Ctrl,            this, &FleetCostDialog::onEditOptions);

    win.pack();
    m_costSummary.requestFocus();
    m_root.centerWidget(win);
    m_root.add(win);
    loop.run();
}

void
FleetCostDialog::render(client::Downlink& link)
{
    game::spec::CostSummary content;
    if (m_teamsActive) {
        m_costProxy.computeFleetCosts(link, game::PlayerSet_t(m_currentTeam), true, content);
        m_label.setText(m_teamSettings.getTeamName(m_currentTeam, m_translator));
    } else {
        m_costProxy.computeFleetCosts(link, game::PlayerSet_t(m_currentPlayer), false, content);
        m_label.setText(m_playerNames.get(m_currentPlayer));
    }
    m_costSummary.setContent(content);
}

void
FleetCostDialog::onEditOptions()
{
    client::Downlink link(m_root, m_translator);
    game::sim::FleetCostOptions opts;
    m_costProxy.getOptions(link, opts);
    bool byTeam = m_teamsActive;

    if (client::dialogs::editSimulationFleetCostOptions(m_root, m_gameSender, opts, m_teamsAvailable ? &byTeam : 0, m_translator)) {
        m_teamsActive = byTeam;
        m_costProxy.setOptions(opts);
        render(link);
    }
}

void
FleetCostDialog::onNext()
{
    browse(+1);
}

void
FleetCostDialog::onPrevious()
{
    browse(-1);
}

void
FleetCostDialog::browse(int delta)
{
    // ex WFleetCostDialog::browse(int delta)
    if (m_teamsActive) {
        const int me = m_teamSettings.getViewpointPlayer();
        m_currentTeam = getNextPlayer(m_involvedTeams, m_currentTeam, delta);
        if (m_involvedPlayers.contains(me) && m_teamSettings.getPlayerTeam(me) == m_currentTeam) {
            // Viewpoint player is part of current team; select them.
            m_currentPlayer = me;
        } else {
            // Pick another player from that team
            for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
                if (m_involvedPlayers.contains(i) && m_teamSettings.getPlayerTeam(i) == m_currentTeam) {
                    m_currentPlayer = i;
                    break;
                }
            }
        }
    } else {
        m_currentPlayer = getNextPlayer(m_involvedPlayers, m_currentPlayer, delta);
        m_currentTeam = m_teamSettings.getPlayerTeam(m_currentPlayer);
    }

    // Render
    client::Downlink link(m_root, m_translator);
    render(link);
}


void
client::dialogs::showSimulationFleetCost(ui::Root& root,
                                         util::RequestSender<game::Session> gameSender,
                                         game::proxy::SimulationSetupProxy& setupProxy,
                                         afl::string::Translator& tx)
{
    // ex ccsim.pas:FleetCostComparator
    FleetCostDialog dlg(root, setupProxy, gameSender, tx);
    if (dlg.init()) {
        dlg.run();
    }
}
