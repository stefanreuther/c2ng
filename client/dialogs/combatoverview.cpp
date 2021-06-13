/**
  *  \file client/dialogs/combatoverview.cpp
  *  \brief Combat Overview dialog
  */

#include "client/dialogs/combatoverview.hpp"
#include "afl/base/deleter.hpp"
#include "client/downlink.hpp"
#include "client/widgets/combatdiagram.hpp"
#include "client/widgets/playerlist.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/teamproxy.hpp"
#include "game/proxy/vcroverviewproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/checkbox.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"

using client::widgets::PlayerList;
using client::widgets::CombatDiagram;

namespace {
    class CombatOverviewDialog {
     public:
        CombatOverviewDialog(ui::Root& root, afl::string::Translator& tx)
            : m_root(root),
              m_translator(tx),
              m_loop(root),
              m_diagram(root, tx),
              m_playerList(root, PlayerList::FlowLayout, PlayerList::ShowNames, PlayerList::PlayerColors,
                           30 * root.provider().getFont("")->getEmWidth(), game::PlayerSet_t()),
              m_useTeamColors(0)
            {
                m_useTeamColors.sig_change.add(this, &CombatOverviewDialog::onUseTeamColors);
                m_diagram.sig_battleClick.add(this, &CombatOverviewDialog::onBattleClick);
                onUseTeamColors();
            }

        bool run()
            {
                // ex WCombatDiagramDialog::init
                afl::string::Translator& tx = m_translator;
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(tx("Combat Overview"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

                win.add(m_diagram);
                win.add(m_playerList);

                ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));

                ui::widgets::Checkbox& colorCheckbox = del.addNew(new ui::widgets::Checkbox(m_root, 't', tx("Team colors"), m_useTeamColors));
                colorCheckbox.addDefaultImages();
                g.add(colorCheckbox);

                // FIXME: port scores
                // if (dynamic_cast<GClassicVcrDatabase*>(&db)) {
                //     g.add(h.add(new UIButton(2, _("Scores"), 's')).addCall(this, &WCombatDiagramDialog::onScore));
                //     g.add(*colorCheckbox);
                // }

                g.add(del.addNew(new ui::Spacer()));

                ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(tx("Close"), util::Key_Return, m_root));
                btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
                g.add(btnClose);
                win.add(g);

                ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
                disp.addNewClosure(util::Key_Escape, m_loop.makeStop(0));
                win.add(disp);
                win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

                gfx::Rectangle area = m_root.getExtent();
                area.grow(-10, -10);
                win.setExtent(area);

                m_root.add(win);
                colorCheckbox.requestFocus();
                return (m_loop.run() != 0);
            }

        void setDiagram(const game::vcr::Overview::Diagram& diag)
            {
                m_diagram.setContent(diag);
            }

        void setVisiblePlayers(game::PlayerSet_t players)
            {
                m_playerList.setVisiblePlayers(players);
            }

        void setPlayerNames(const game::PlayerArray<String_t>& names)
            {
                m_playerList.setNames(names);
            }

        void setTeams(const game::TeamSettings& teams)
            {
                m_diagram.setTeams(teams);
            }

        void onBattleClick(size_t index)
            {
                m_chosenBattle = index;
                m_loop.stop(1);
            }

        void onUseTeamColors()
            {
                m_diagram.setUseTeamColors(m_useTeamColors.get() != 0);
            }

        size_t getChosenBattle() const
            {
                return m_chosenBattle;
            }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::EventLoop m_loop;
        CombatDiagram m_diagram;
        PlayerList m_playerList;
        afl::base::Observable<int> m_useTeamColors;
        size_t m_chosenBattle;
    };


    void initDialog(client::Downlink& link, CombatOverviewDialog& dlg, const util::RequestSender<game::Session>& gameSender)
    {
        // Players
        {
            game::proxy::PlayerProxy pp(gameSender);
            dlg.setVisiblePlayers(pp.getAllPlayers(link));
            dlg.setPlayerNames(pp.getPlayerNames(link, game::Player::AdjectiveName));
        }

        // Teams
        {
            game::proxy::TeamProxy tp(gameSender);
            game::TeamSettings teams;
            tp.init(link, teams);
            dlg.setTeams(teams);
        }
    }
}

bool
client::dialogs::showCombatOverview(ui::Root& root,
                                    afl::string::Translator& tx,
                                    util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender,
                                    util::RequestSender<game::Session> gameSender,
                                    size_t& chosenBattle)
{
    // ex doCombatDialog
    // Build diagram
    game::proxy::VcrOverviewProxy proxy(vcrSender);
    Downlink link(root, tx);
    game::vcr::Overview::Diagram diag;
    proxy.buildDiagram(link, diag);

    // Need at least two battles
    if (diag.battles.size() < 2) {
        return false;
    }

    // Display the dialog
    CombatOverviewDialog dlg(root, tx);
    dlg.setDiagram(diag);
    initDialog(link, dlg, gameSender);
    bool ok = dlg.run();
    if (ok) {
        chosenBattle = dlg.getChosenBattle();
    }
    return ok;
}
