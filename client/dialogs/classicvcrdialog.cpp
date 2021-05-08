/**
  *  \file client/dialogs/classicvcrdialog.cpp
  */

#include "client/dialogs/classicvcrdialog.hpp"
#include "client/dialogs/classicvcrobject.hpp"
#include "client/downlink.hpp"
#include "client/picturenamer.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/unicodechars.hpp"


client::dialogs::ClassicVcrDialog::ClassicVcrDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, util::RequestSender<game::Session> gameSender)
    : m_root(root),
      m_translator(tx),
      m_proxy(vcrSender, root.engine().dispatcher(), tx, std::auto_ptr<game::spec::info::PictureNamer>(new client::PictureNamer())),
      m_gameSender(gameSender),
      m_info(root),
      m_loop(root),
      m_result(),
      m_currentIndex(0),
      m_numBattles(0)
{
    m_proxy.sig_update.add(this, &ClassicVcrDialog::onUpdate);
    m_info.sig_left.add(this, &ClassicVcrDialog::onLeftInfo);
    m_info.sig_right.add(this, &ClassicVcrDialog::onRightInfo);
}

client::dialogs::ClassicVcrDialog::~ClassicVcrDialog()
{ }

game::Reference
client::dialogs::ClassicVcrDialog::run()
{
    // Query number of battles
    initNumBattles();
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

    btnUp.sig_fire.add(this, &ClassicVcrDialog::onPrevious);
    btnDown.sig_fire.add(this, &ClassicVcrDialog::onNext);
    btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnPlay.sig_fire.add(this, &ClassicVcrDialog::onPlay);

    postLoad();

    window.pack();
    m_root.centerWidget(window);
    m_root.add(window);
    m_loop.run();

    return m_result;
}

void
client::dialogs::ClassicVcrDialog::initNumBattles()
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
}

void
client::dialogs::ClassicVcrDialog::onPrevious()
{
    // ex WVcrSelector::next
    if (m_currentIndex > 0) {
        setCurrentIndex(m_currentIndex-1);
    }
}

void
client::dialogs::ClassicVcrDialog::onNext()
{
    // ex WVcrSelector::prev
    if (m_numBattles - m_currentIndex > 1) {
        setCurrentIndex(m_currentIndex+1);
    }
}

void
client::dialogs::ClassicVcrDialog::onPlay()
{
    sig_play.raise(m_currentIndex);
}

void
client::dialogs::ClassicVcrDialog::setCurrentIndex(size_t index)
{
    m_currentIndex = index;
    postLoad();
}

void
client::dialogs::ClassicVcrDialog::postLoad()
{
    m_proxy.setCurrentBattle(m_currentIndex);
}

void
client::dialogs::ClassicVcrDialog::onUpdate(size_t /*index*/, const game::vcr::BattleInfo& data)
{
    m_info.setData(data);
}

void
client::dialogs::ClassicVcrDialog::onLeftInfo()
{
    onSideInfo(0);
}

void
client::dialogs::ClassicVcrDialog::onRightInfo()
{
    onSideInfo(1);
}

void
client::dialogs::ClassicVcrDialog::onSideInfo(size_t side)
{
    m_result = doClassicVcrObjectInfoDialog(m_root, m_translator, m_gameSender, m_proxy, side);
    if (m_result.isSet()) {
        m_loop.stop(1);
    }
}
