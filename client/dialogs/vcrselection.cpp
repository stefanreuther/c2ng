/**
  *  \file client/dialogs/vcrselection.cpp
  *  \brief Class client::dialogs::VcrSelection
  */

#include "client/dialogs/vcrselection.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/classicvcrobject.hpp"
#include "client/dialogs/combatoverview.hpp"
#include "client/dialogs/combatscoresummary.hpp"
#include "client/dialogs/export.hpp"
#include "client/dialogs/flakvcrobject.hpp"
#include "client/dialogs/sessionfileselectiondialog.hpp"
#include "client/downlink.hpp"
#include "client/picturenamer.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/teamproxy.hpp"
#include "game/proxy/vcrexportadaptor.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"
#include "util/unicodechars.hpp"

using afl::string::Format;
using client::dialogs::SessionFileSelectionDialog;
using client::widgets::VcrInfo;
using game::proxy::VcrDatabaseProxy;
using ui::dialogs::MessageBox;
using util::FileNamePattern;

client::dialogs::VcrSelection::VcrSelection(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, util::RequestSender<game::Session> gameSender)
    : m_root(root),
      m_translator(tx),
      m_proxy(vcrSender, root.engine().dispatcher(), tx, std::auto_ptr<game::spec::info::PictureNamer>(new client::PictureNamer())),
      m_vcrSender(vcrSender),
      m_gameSender(gameSender),
      m_info(root, tx),
      m_loop(root),
      m_result(),
      m_battleInfo(),
      m_currentIndex(0),
      m_numBattles(0),
      m_kind()
{
    m_proxy.sig_update.add(this, &VcrSelection::onUpdate);
    m_info.sig_action.add(this, &VcrSelection::onAction);
    m_info.sig_showMap.add(this, &VcrSelection::onShowMap);
}

client::dialogs::VcrSelection::~VcrSelection()
{ }

game::Reference
client::dialogs::VcrSelection::run()
{
    // Query number of battles
    init();
    if (m_numBattles == 0) {
        return game::Reference();
    }

    // Build dialog
    ui::Window window(m_translator("VCR"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
    window.add(m_info);

    client::widgets::HelpWidget help(m_root, m_translator, m_gameSender, "pcc2:vcr");
    ui::widgets::Button btnUp(UTF_UP_ARROW, util::Key_Up, m_root);
    ui::widgets::Button btnDown(UTF_DOWN_ARROW, util::Key_Down, m_root);
    ui::widgets::Button btnPlay(m_translator("Play"), util::Key_Return, m_root);
    ui::Spacer spc;
    ui::widgets::Button btnCancel(m_translator("Back"), util::Key_Escape, m_root);
    ui::widgets::Button btnHelp(m_translator("Help"), 'h', m_root);

    ui::Group g(ui::layout::HBox::instance5);
    g.add(btnUp);
    g.add(btnDown);
    g.add(btnPlay);
    g.add(spc);
    g.add(btnHelp);
    g.add(btnCancel);
    window.add(g);

    ui::widgets::Quit quit(m_root, m_loop);
    window.add(quit);
    window.add(help);

    btnUp.sig_fire.add(this, &VcrSelection::onPrevious);
    btnDown.sig_fire.add(this, &VcrSelection::onNext);
    btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnPlay.sig_fire.add(this, &VcrSelection::onPlay);
    btnHelp.dispatchKeyTo(help);

    // Extra keys
    ui::widgets::KeyDispatcher disp;
    window.add(disp);
    disp.add('-',                                this, &VcrSelection::onPrevious);
    disp.add(util::Key_WheelUp,                  this, &VcrSelection::onPrevious);
    disp.add(util::Key_PgUp,                     this, &VcrSelection::onPrevious);
    disp.add('+',                                this, &VcrSelection::onNext);
    disp.add(util::Key_WheelDown,                this, &VcrSelection::onNext);
    disp.add(util::Key_PgDn,                     this, &VcrSelection::onNext);
    disp.add(util::Key_Home,                     this, &VcrSelection::onFirst);
    disp.add(util::Key_Home + util::KeyMod_Ctrl, this, &VcrSelection::onFirst);
    disp.add(util::Key_PgUp + util::KeyMod_Ctrl, this, &VcrSelection::onFirst);
    disp.add(util::Key_End,                      this, &VcrSelection::onLast);
    disp.add(util::Key_End + util::KeyMod_Ctrl,  this, &VcrSelection::onLast);
    disp.add(util::Key_PgDn + util::KeyMod_Ctrl, this, &VcrSelection::onLast);

    postLoad();

    window.pack();
    m_root.centerWidget(window);
    m_root.add(window);
    m_loop.run();

    return m_result;
}

void
client::dialogs::VcrSelection::init()
{
    Downlink link(m_root, m_translator);
    VcrDatabaseProxy::Status st;
    m_proxy.getStatus(link, st);

    m_numBattles = st.numBattles;
    m_currentIndex = st.currentBattle;
    m_kind = st.kind;

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
client::dialogs::VcrSelection::onPrevious()
{
    if (m_currentIndex > 0) {
        setCurrentIndex(m_currentIndex-1);
    }
}

void
client::dialogs::VcrSelection::onNext()
{
    // ex FlakVcrSelector::next
    if (m_numBattles - m_currentIndex > 1) {
        setCurrentIndex(m_currentIndex+1);
    }
}

void
client::dialogs::VcrSelection::onFirst()
{
    // ex FlakVcrSelector::previous
    if (m_currentIndex != 0) {
        setCurrentIndex(0);
    }
}

void
client::dialogs::VcrSelection::onLast()
{
    if (m_currentIndex != m_numBattles-1) {
        setCurrentIndex(m_numBattles-1);
    }
}

void
client::dialogs::VcrSelection::onPlay()
{
    sig_play.raise(m_currentIndex);
}

void
client::dialogs::VcrSelection::setCurrentIndex(size_t index)
{
    // ex FlakVcrSelector::setCurrent
    m_currentIndex = index;
    postLoad();
}

void
client::dialogs::VcrSelection::postLoad()
{
    m_proxy.setCurrentBattle(m_currentIndex);
}

void
client::dialogs::VcrSelection::onUpdate(size_t /*index*/, const game::vcr::BattleInfo& data)
{
    m_battleInfo = data;
    m_info.setData(data);
}

void
client::dialogs::VcrSelection::onInfo(size_t index)
{
    // ex FlakVcrSelector::showInfo
    if (!m_battleInfo.groups.empty()) {
        switch (m_kind) {
         case VcrDatabaseProxy::UnknownCombat:
            break;

         case VcrDatabaseProxy::ClassicCombat:
            m_result = doClassicVcrObjectInfoDialog(m_root, m_translator, m_gameSender, m_proxy, index);
            break;

         case VcrDatabaseProxy::FlakCombat:
            m_result = doFlakVcrObjectInfoDialog(m_root, m_translator, m_gameSender, m_proxy, m_battleInfo, index);
            break;
        }

        if (m_result.isSet()) {
            m_loop.stop(1);
        }
    }
}

void
client::dialogs::VcrSelection::onAction(client::widgets::VcrInfo::Action a)
{
    switch (a) {
     case VcrInfo::ShowCombatDiagram:
        onTab();
        break;
     case VcrInfo::ShowScoreSummary:
        showCombatScoreSummary(m_root, m_translator, m_vcrSender, m_gameSender);
        break;
     case VcrInfo::ExportBattles:
        doExport(m_root, m_vcrSender.makeTemporary(game::proxy::makeVcrExportAdaptor()), m_gameSender, m_translator);
        break;
     case VcrInfo::ExportUnits:
        doExport(m_root, m_vcrSender.makeTemporary(game::proxy::makeVcrSideExportAdaptor(m_currentIndex)), m_gameSender, m_translator);
        break;
     case VcrInfo::SaveAllBattles:
        onSave(0, m_numBattles);
        break;
     case VcrInfo::SaveThisBattle:
        onSave(m_currentIndex, 1);
        break;
    }
}

void
client::dialogs::VcrSelection::onTab()
{
    // ex FlakVcrSelector::showDiagram
    size_t pos = 0;
    bool ok = showCombatOverview(m_root, m_translator, m_vcrSender, m_gameSender, pos);
    if (ok) {
        setCurrentIndex(pos);
    }
}

void
client::dialogs::VcrSelection::onShowMap(game::map::Point pt)
{
    m_result = pt;
    m_loop.stop(1);
}

void
client::dialogs::VcrSelection::onSave(size_t first, size_t num)
{
    client::Downlink link(m_root, m_translator);
    SessionFileSelectionDialog dlg(m_root, m_translator, m_gameSender, m_translator("Save"));
    dlg.setPattern(FileNamePattern::getAllFilesWithExtensionPattern("vcr"));
    dlg.setDefaultExtension("vcr");
    if (dlg.runDefault(link)) {
        String_t name = dlg.getResult();
        String_t err;
        if (!m_proxy.save(link, name, first, num, err)) {
            MessageBox(Format(m_translator("Error during save: %s"), err), m_translator("Save"), m_root)
                .doOkDialog(m_translator);
        }
    }
}
