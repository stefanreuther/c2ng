/**
  *  \file client/dialogs/messagereceiver.cpp
  *  \brief Class client::dialogs::MessageReceiver
  */

#include "client/dialogs/messagereceiver.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/statictext.hpp"

client::dialogs::MessageReceiver::MessageReceiver(String_t title, client::widgets::PlayerSetSelector& sel, ui::Root& root, afl::string::Translator& tx)
    : Window(title, root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5),
      m_root(root),
      m_selector(sel),
      m_translator(tx),
      m_deleter(),
      m_loop(root),
      m_actionGroup(m_deleter.addNew(new ui::Group(ui::layout::HBox::instance5))),
      m_actionSpacer(m_deleter.addNew(new ui::Spacer())),
      m_buttons(m_deleter.addNew(new ui::widgets::StandardDialogButtons(root, tx))),
      m_universalSet(),
      conn_setChange(sel.sig_setChange.add(this, &MessageReceiver::onSetChange))
{
    // ex WNewMessageReceiverWindow::init
    add(m_deleter.addNew(new ui::widgets::StaticText(tx("Message receivers"), util::SkinColor::Static, gfx::FontRequest(), root.provider())));

    add(ui::widgets::FrameGroup::wrapWidget(m_deleter,
                                            root.colorScheme(),
                                            ui::LoweredFrame,
                                            m_deleter.addNew(new ui::widgets::ScrollbarContainer(sel, root))));

    add(m_actionGroup);
    add(m_buttons);
    add(m_deleter.addNew(new ui::widgets::Quit(root, m_loop)));

    m_buttons.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    m_buttons.ok().sig_fire.add(this, &MessageReceiver::onOK);

    m_actionGroup.add(m_actionSpacer);

    onSetChange();
}

client::dialogs::MessageReceiver&
client::dialogs::MessageReceiver::addUniversalToggle(game::PlayerSet_t set)
{
    ui::widgets::Button& btn = m_deleter.addNew(new ui::widgets::Button(m_translator("Universal"), 'u', m_root));
    m_actionGroup.addChild(btn, m_actionSpacer.getPreviousSibling());
    btn.sig_fire.add(this, &MessageReceiver::onToggleUniversal);
    m_universalSet = set;
    return *this;
}

client::dialogs::MessageReceiver&
client::dialogs::MessageReceiver::addExtra(util::KeyString label, int code)
{
    ui::widgets::Button& btn = m_deleter.addNew(new ui::widgets::Button(label, m_root));
    m_actionGroup.addChild(btn, m_actionSpacer.getPreviousSibling());
    btn.sig_fire.addNewClosure(m_loop.makeStop(code));
    return *this;
}

client::dialogs::MessageReceiver&
client::dialogs::MessageReceiver::addHelp(ui::Widget& helper)
{
    m_buttons.addHelp(helper);
    return *this;
}

int
client::dialogs::MessageReceiver::run()
{
    m_root.add(*this);
    int n = m_loop.run();
    m_root.removeChild(*this);
    return n;
}

void
client::dialogs::MessageReceiver::onOK()
{
    if (!m_selector.getSelectedPlayers().empty()) {
        m_loop.stop(1);
    }
}

void
client::dialogs::MessageReceiver::onToggleUniversal()
{
    m_selector.togglePlayers(m_universalSet);
}

void
client::dialogs::MessageReceiver::onSetChange()
{
    m_buttons.ok().setState(DisabledState, m_selector.getSelectedPlayers().empty());
}
