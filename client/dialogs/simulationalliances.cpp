/**
  *  \file client/dialogs/simulationalliances.cpp
   *  \brief Alliance Editor for Battle Simulator
 */

#include "client/dialogs/simulationalliances.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/playerlist.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/sim/configuration.hpp"
#include "ui/cardgroup.hpp"
#include "ui/eventloop.hpp"
#include "ui/icons/colortile.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/cardtabbar.hpp"
#include "ui/widgets/checkbox.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/icongrid.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/rich/parser.hpp"

using afl::base::Observable;
using client::widgets::PlayerList;
using game::PlayerArray;
using game::PlayerBitMatrix;
using game::PlayerSet_t;
using game::proxy::SimulationSetupProxy;
using game::sim::Configuration;
using ui::widgets::Checkbox;
using ui::widgets::FocusIterator;
using ui::widgets::KeyDispatcher;

namespace {
    /* Get grid cell size.
       Each cell is a square of current line height (minus one for the cursor frame). */
    gfx::Point getCellSize(ui::Root& root)
    {
        int dim = root.provider().getFont(gfx::FontRequest())->getLineHeight() - 1;
        return gfx::Point(dim, dim);
    }

    /* Given a possibly-sparse set of players, get the n-th element. */
    int getPlayerNumberFromIndex(PlayerSet_t allPlayers, int index)
    {
        for (int i = 0; i <= game::MAX_PLAYERS; ++i) {
            if (allPlayers.contains(i)) {
                if (index == 0) {
                    return i;
                }
                --index;
            }
        }
        return 0;
    }


    /*
     *  Page - represents one of the dialog's pages (icon grid plus decoration/explanation)
     */

    class Page {
        // ex WSimAllianceGrid (sort-of)
     public:
        Page(String_t info, const PlayerArray<String_t>& playerNames, PlayerSet_t allPlayers, uint8_t color, ui::Root& root);

        void setContent(const PlayerBitMatrix& currentSettings, const PlayerBitMatrix& defaultSettings);
        void usePlayerRelations();
        void setSymmetric(bool flag);
        const PlayerBitMatrix& getCurrentSettings() const;
        ui::Widget& build(afl::base::Deleter& del);
        void renderInfo(afl::string::Translator& tx);

        afl::base::Signal<void()> sig_change;

     private:
        const String_t m_info;
        const PlayerSet_t m_allPlayers;
        ui::icons::ColorTile m_activeTile;
        ui::icons::ColorTile m_inactiveTile;
        ui::widgets::IconGrid m_grid;
        PlayerList m_horizontalList;
        PlayerList m_verticalList;
        PlayerBitMatrix m_currentSettings;
        PlayerBitMatrix m_defaultSettings;
        ui::rich::DocumentView m_infoView;
        bool m_symmetric;

        void onClick();
        void render();
    };

    class Dialog {
     public:
        Dialog(const PlayerArray<String_t>& playerNames, PlayerSet_t allPlayers, ui::Root& root, afl::string::Translator& tx);

        void setContent(const Configuration& config, const SimulationSetupProxy::PlayerRelations& rel);
        void updateConfiguration(Configuration& config) const;
        bool isUsePlayerRelations() const;
        bool run(util::RequestSender<game::Session> gameSender, ui::Root& root, afl::string::Translator& tx);

     private:
        // Event handlers:
        void onContentEdited();
        void onSymmetricChange();
        void onUsePlayerRelationsChange();

        Page m_alliancePage;
        Page m_enemyPage;
        Observable<int> m_symmetric;
        Observable<int> m_usePlayerRelations;
        ui::CardGroup m_cardGroup;
    };
}


/*
 *  Page
 */

Page::Page(String_t info, const PlayerArray<String_t>& playerNames, PlayerSet_t allPlayers, uint8_t color, ui::Root& root)
    : sig_change(),
      m_info(info),
      m_allPlayers(allPlayers),
      m_activeTile(root, getCellSize(root), color),
      m_inactiveTile(root, getCellSize(root), ui::Color_Grayscale + 9),
      m_grid(root.engine(), getCellSize(root), static_cast<int>(allPlayers.size()), static_cast<int>(allPlayers.size())),
      m_horizontalList(root, PlayerList::HorizontalLayout, PlayerList::ShowLetters, PlayerList::SameColors, 0, allPlayers),
      m_verticalList(root,   PlayerList::VerticalLayout,   PlayerList::ShowNames,   PlayerList::SameColors, 0, allPlayers),
      m_infoView(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(15, 10), 0, root.provider()),
      m_symmetric()
{
    m_horizontalList.setNames(playerNames);
    m_verticalList.setNames(playerNames);
    m_grid.sig_doubleClick.add(this, &Page::onClick);
}

/* Set initial content. */
void
Page::setContent(const PlayerBitMatrix& currentSettings, const PlayerBitMatrix& defaultSettings)
{
    // ex WSimAllianceEditor::setAllianceData(), WSimAllianceEditor::setEnemyData()
    m_currentSettings = currentSettings;
    m_defaultSettings = defaultSettings;
    render();
    m_grid.setCurrentItem(1);
}

/* Handle activation of "use default relations". */
void
Page::usePlayerRelations()
{
    m_currentSettings = m_defaultSettings;
    render();
}

/* Handle change of "symmetric relations". */
void
Page::setSymmetric(bool flag)
{
    m_symmetric = flag;
}

/* Get current settings. For write-back to game. */
const PlayerBitMatrix&
Page::getCurrentSettings() const
{
    return m_currentSettings;
}

/* Build the page's widgets. */
ui::Widget&
Page::build(afl::base::Deleter& del)
{
    // ex WSimAllianceEditor::makeGrid
    ui::Group& g = del.addNew(new ui::Group(del.addNew(new ui::layout::Grid(3))));
    g.add(del.addNew(new ui::Spacer()));
    g.add(m_horizontalList);
    g.add(del.addNew(new ui::Spacer()));
    g.add(m_verticalList);
    g.add(m_grid);
    g.add(m_infoView);
    m_grid.requestFocus();

    KeyDispatcher& disp = del.addNew(new KeyDispatcher());
    disp.add(' ', this, &Page::onClick);
    g.add(disp);

    return g;
}

/* Render information.
   Call after layout so it sees correct width of the multi-line text. */
void
Page::renderInfo(afl::string::Translator& tx)
{
    ui::rich::Document& doc = m_infoView.getDocument();
    doc.add(m_info);
    doc.addParagraph();
    doc.add(util::rich::Parser::parseXml(tx("<kbd>Ctrl+Tab</kbd> to switch page.")));
    doc.addNewline();
    doc.finish();
    m_infoView.handleDocumentUpdate();
}

/* Event handler: click into grid (=value toggle). */
void
Page::onClick()
{
    // ex WSimAllianceEditor::onClick(int pl, int ally, GPlayerBitMatrix& mtx)
    const int pl   = getPlayerNumberFromIndex(m_allPlayers, m_grid.getCurrentLine());
    const int ally = getPlayerNumberFromIndex(m_allPlayers, m_grid.getCurrentColumn());
    const bool value = !m_currentSettings.get(pl, ally);
    m_currentSettings.set(pl, ally, value);
    if (m_symmetric) {
        m_currentSettings.set(ally, pl, value);
    }
    render();
    sig_change.raise();
}

/* Render content. */
void
Page::render()
{
    // WSimAllianceGrid::setContent, sort-of
    int y = 0;
    for (int py = 0; py <= game::MAX_PLAYERS; ++py) {
        if (m_allPlayers.contains(py)) {
            int x = 0;
            for (int px = 0; px <= game::MAX_PLAYERS; ++px) {
                if (m_allPlayers.contains(px)) {
                    if (px == py) {
                        m_grid.setIcon(x, y, 0);
                        m_grid.setItemAccessible(x, y, false);
                    } else {
                        m_grid.setIcon(x, y, m_currentSettings.get(py, px) ? &m_activeTile : &m_inactiveTile);
                        m_grid.setItemAccessible(x, y, true);
                    }
                    ++x;
                }
            }
            ++y;
        }
    }
}


/*
 *  Dialog
 */

Dialog::Dialog(const PlayerArray<String_t>& playerNames, PlayerSet_t allPlayers, ui::Root& root, afl::string::Translator& tx)
    : m_alliancePage(tx("If a player (row) offers an alliance to another one (column), "
                        "their units will not attack the ally's units."),
                     playerNames, allPlayers, ui::Color_GreenScale + 7, root),
      m_enemyPage(tx("If a player (row) declares another one (column) their enemy, "
                     "ships with Primary Enemy set will behave as if the enemy were an additional Primary Enemy."),
                  playerNames, allPlayers, ui::Color_DarkRed, root),
      m_symmetric(1),
      m_usePlayerRelations(0),
      m_cardGroup()
{
    m_alliancePage.sig_change.add(this, &Dialog::onContentEdited);
    m_enemyPage.sig_change.add(this, &Dialog::onContentEdited);
    m_symmetric.sig_change.add(this, &Dialog::onSymmetricChange);
    m_usePlayerRelations.sig_change.add(this, &Dialog::onUsePlayerRelationsChange);
}

/* Set content with values obtained from game/proxy. */
void
Dialog::setContent(const Configuration& config, const SimulationSetupProxy::PlayerRelations& rel)
{
    m_alliancePage.setContent(config.allianceSettings(), rel.alliances);
    m_enemyPage.setContent(config.enemySettings(), rel.enemies);
    m_usePlayerRelations.set(rel.usePlayerRelations);
    onSymmetricChange();
    onUsePlayerRelationsChange();
}

/* Update config with new alliance/enemy values. */
void
Dialog::updateConfiguration(Configuration& config) const
{
    config.allianceSettings() = m_alliancePage.getCurrentSettings();
    config.enemySettings() = m_enemyPage.getCurrentSettings();
}

/* Get current value of "use default relations" setting. */
bool
Dialog::isUsePlayerRelations() const
{
    return m_usePlayerRelations.get() != 0;
}

/* Main entry point. */
bool
Dialog::run(util::RequestSender<game::Session> gameSender, ui::Root& root, afl::string::Translator& tx)
{
    // ex WSimAllianceEditor::run()
    // VBox
    //   CardTabBar
    //   CardGroup
    //   Checkbox "symmetric"
    //   Checkbox "auto"
    //   StandardDialogButtons
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(tx("Edit Alliances"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // CardTabBar and CardGroup
    ui::widgets::CardTabBar& bar = del.addNew(new ui::widgets::CardTabBar(root, m_cardGroup));
    ui::Widget& a = m_alliancePage.build(del);
    ui::Widget& e = m_enemyPage.build(del);

    m_cardGroup.add(a);
    m_cardGroup.add(e);
    bar.addPage(tx("Alliances"), 'a', a);
    bar.addPage(tx("Enemies"),   'e', e);
    bar.setKeys(ui::widgets::CardTabBar::CtrlTab + ui::widgets::CardTabBar::F6);
    win.add(bar);
    win.add(m_cardGroup);

    // Checkboxes
    Checkbox& symCB = del.addNew(new Checkbox(root, 's', tx("Symmetric relations"), m_symmetric));
    symCB.addDefaultImages();
    win.add(symCB);

    Checkbox& useCB = del.addNew(new Checkbox(root, 'u', tx("Use default/game relations"), m_usePlayerRelations));
    useCB.addDefaultImages();
    win.add(useCB);

    // Buttons/admin
    ui::EventLoop loop(root);
    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(root, tx));
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(root, tx, gameSender, "pcc2:simallies"));
    btn.addStop(loop);
    btn.addHelp(help);
    win.add(help);
    win.add(btn);
    win.add(del.addNew(new ui::widgets::Quit(root, loop)));

    // FocusIterator. When focused, IconGrid will swallow up/down arrows.
    FocusIterator& it = del.addNew(new FocusIterator(FocusIterator::Tab | FocusIterator::Vertical));
    it.add(m_cardGroup);
    it.add(symCB);
    it.add(useCB);
    win.add(it);

    // Final layout. renderInfo() must be after pack() so DocumentView sees correct width.
    win.pack();
    m_alliancePage.renderInfo(tx);
    m_enemyPage.renderInfo(tx);
    m_cardGroup.requestFocus();
    root.centerWidget(win);
    root.add(win);

    // Run
    return loop.run() != 0;
}

/* Event handler: content edited.
   Must reset the "use default relations" flag. */
void
Dialog::onContentEdited()
{
    m_usePlayerRelations.set(0);
}

/* Event handler: "symmetric" setting changed.
   Must propagate to pages. */
void
Dialog::onSymmetricChange()
{
    m_alliancePage.setSymmetric(m_symmetric.get() != 0);
    m_enemyPage.setSymmetric(m_symmetric.get() != 0);
}

/* Event handler: "use default relations" flag changed.
   If it gets enabled, must set actual defaults. */
void
Dialog::onUsePlayerRelationsChange()
{
    // ex WSimAllianceEditor::onAutoClick()
    if (m_usePlayerRelations.get() != 0) {
        m_alliancePage.usePlayerRelations();
        m_enemyPage.usePlayerRelations();
    }
}


/*
 *  Main Entry Point
 */

void
client::dialogs::editAlliances(game::proxy::SimulationSetupProxy& proxy,
                               util::RequestSender<game::Session> gameSender,
                               ui::Root& root,
                               afl::string::Translator& tx)
{
    // ex editAlliances(GSimOptions& opts, bool& sync, GSimulatorGameInterface& game_if)
    // Retrieve initial status
    Downlink link(root, tx);

    SimulationSetupProxy::PlayerRelations rel;
    Configuration config;
    proxy.getPlayerRelations(link, rel);
    proxy.getConfiguration(link, config);

    game::proxy::PlayerProxy playerProxy(gameSender);
    PlayerArray<String_t> playerNames = playerProxy.getPlayerNames(link, game::Player::ShortName);
    PlayerSet_t allPlayers = playerProxy.getAllPlayers(link);

    // Dialog
    Dialog dlg(playerNames, allPlayers, root, tx);
    dlg.setContent(config, rel);
    if (dlg.run(gameSender, root, tx)) {
        dlg.updateConfiguration(config);
        proxy.setConfiguration(config, Configuration::Areas_t() + Configuration::AllianceArea + Configuration::EnemyArea);
        proxy.setUsePlayerRelations(dlg.isUsePlayerRelations());
    }
}
