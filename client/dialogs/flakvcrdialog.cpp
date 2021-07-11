/**
  *  \file client/dialogs/flakvcrdialog.cpp
  *  \brief Class client::dialogs::FlakVcrDialog
  */

#include "client/dialogs/flakvcrdialog.hpp"
#include "client/dialogs/combatoverview.hpp"
#include "client/dialogs/combatscoresummary.hpp"
#include "client/downlink.hpp"
#include "client/picturenamer.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/teamproxy.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"
#include "util/unicodechars.hpp"

client::dialogs::FlakVcrDialog::FlakVcrDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, util::RequestSender<game::Session> gameSender)
    : m_root(root),
      m_translator(tx),
      m_proxy(vcrSender, root.engine().dispatcher(), tx, std::auto_ptr<game::spec::info::PictureNamer>(new client::PictureNamer())),
      m_vcrSender(vcrSender),
      m_gameSender(gameSender),
      m_info(root, tx),
      m_loop(root),
      m_result(),
      m_currentIndex(0),
      m_numBattles(0)
{
    m_proxy.sig_update.add(this, &FlakVcrDialog::onUpdate);
    m_info.sig_list.add(this, &FlakVcrDialog::onList);
    m_info.sig_tab.add(this, &FlakVcrDialog::onTab);
    m_info.sig_score.add(this, &FlakVcrDialog::onScore);
}

client::dialogs::FlakVcrDialog::~FlakVcrDialog()
{ }

game::Reference
client::dialogs::FlakVcrDialog::run()
{
    // Query number of battles
    init();
    if (m_numBattles == 0) {
        return game::Reference();
    }

    // Build dialog
    ui::Window window(m_translator("VCR"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
    window.add(m_info);

    ui::widgets::Button btnUp(UTF_UP_ARROW, util::Key_Up, m_root);
    ui::widgets::Button btnDown(UTF_DOWN_ARROW, util::Key_Down, m_root);
    ui::widgets::Button btnPlay(m_translator("Play"), util::Key_Return, m_root);
    ui::Spacer spc;
    ui::widgets::Button btnCancel(m_translator("Back"), util::Key_Escape, m_root);

    ui::Group g(ui::layout::HBox::instance5);
    g.add(btnUp);
    g.add(btnDown);
    g.add(btnPlay);
    g.add(spc);
    g.add(btnCancel);
    window.add(g);

    ui::widgets::Quit quit(m_root, m_loop);
    window.add(quit);

    btnUp.sig_fire.add(this, &FlakVcrDialog::onPrevious);
    btnDown.sig_fire.add(this, &FlakVcrDialog::onNext);
    btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnPlay.sig_fire.add(this, &FlakVcrDialog::onPlay);

    // Extra keys
    ui::widgets::KeyDispatcher disp;
    window.add(disp);
    disp.add('-',                                this, &FlakVcrDialog::onPrevious);
    disp.add(util::Key_WheelUp,                  this, &FlakVcrDialog::onPrevious);
    disp.add(util::Key_PgUp,                     this, &FlakVcrDialog::onPrevious);
    disp.add('+',                                this, &FlakVcrDialog::onNext);
    disp.add(util::Key_WheelDown,                this, &FlakVcrDialog::onNext);
    disp.add(util::Key_PgDn,                     this, &FlakVcrDialog::onNext);
    disp.add(util::Key_Home,                     this, &FlakVcrDialog::onFirst);
    disp.add(util::Key_Home + util::KeyMod_Ctrl, this, &FlakVcrDialog::onFirst);
    disp.add(util::Key_PgUp + util::KeyMod_Ctrl, this, &FlakVcrDialog::onFirst);
    disp.add(util::Key_End,                      this, &FlakVcrDialog::onLast);
    disp.add(util::Key_End + util::KeyMod_Ctrl,  this, &FlakVcrDialog::onLast);
    disp.add(util::Key_PgDn + util::KeyMod_Ctrl, this, &FlakVcrDialog::onLast);

    postLoad();

    window.pack();
    m_root.centerWidget(window);
    m_root.add(window);
    m_loop.run();

    return m_result;
}

void
client::dialogs::FlakVcrDialog::init()
{
    Downlink link(m_root, m_translator);
    game::proxy::VcrDatabaseProxy::Status st;
    m_proxy.getStatus(link, st);

    m_numBattles = st.numBattles;
    m_currentIndex = st.currentBattle;

    // Process result
    if (m_currentIndex >= m_numBattles) {
        m_currentIndex = 0;
    }

    // Populate info view
    // - names
    m_info.setPlayerNames(game::proxy::PlayerProxy(m_gameSender).getPlayerNames(link, game::Player::AdjectiveName));

    // - teams
    game::TeamSettings teams;
    game::proxy::TeamProxy(m_gameSender).init(link, teams);
    m_info.setTeams(teams);

    // - tab
    m_info.setTabAvailable(m_numBattles > 1);
}

void
client::dialogs::FlakVcrDialog::onPrevious()
{
    if (m_currentIndex > 0) {
        setCurrentIndex(m_currentIndex-1);
    }
}

void
client::dialogs::FlakVcrDialog::onNext()
{
    // ex FlakVcrSelector::next
    if (m_numBattles - m_currentIndex > 1) {
        setCurrentIndex(m_currentIndex+1);
    }
}

void
client::dialogs::FlakVcrDialog::onFirst()
{
    // ex FlakVcrSelector::previous
    if (m_currentIndex != 0) {
        setCurrentIndex(0);
    }
}

void
client::dialogs::FlakVcrDialog::onLast()
{
    if (m_currentIndex != m_numBattles-1) {
        setCurrentIndex(m_numBattles-1);
    }
}

void
client::dialogs::FlakVcrDialog::onPlay()
{
    sig_play.raise(m_currentIndex);
}

void
client::dialogs::FlakVcrDialog::setCurrentIndex(size_t index)
{
    // ex FlakVcrSelector::setCurrent
    m_currentIndex = index;
    postLoad();
}

void
client::dialogs::FlakVcrDialog::postLoad()
{
    m_proxy.setCurrentBattle(m_currentIndex);
}

void
client::dialogs::FlakVcrDialog::onUpdate(size_t /*index*/, const game::vcr::BattleInfo& data)
{
    m_info.setData(data);
}

void
client::dialogs::FlakVcrDialog::onList()
{
    // ex FlakVcrSelector::showInfo
    // FIXME: missing: participant list
}

void
client::dialogs::FlakVcrDialog::onTab()
{
    // ex FlakVcrSelector::showDiagram
    size_t pos = 0;
    bool ok = showCombatOverview(m_root, m_translator, m_vcrSender, m_gameSender, pos);
    if (ok) {
        setCurrentIndex(pos);
    }
}

void
client::dialogs::FlakVcrDialog::onScore()
{
    showCombatScoreSummary(m_root, m_translator, m_vcrSender, m_gameSender);
}

