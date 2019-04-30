/**
  *  \file client/dialogs/friendlycodedialog.cpp
  */

#include "client/dialogs/friendlycodedialog.hpp"
#include "ui/window.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/group.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/statictext.hpp"
#include "util/translation.hpp"
#include "ui/spacer.hpp"
#include "ui/eventloop.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/button.hpp"
#include "client/downlink.hpp"
#include "client/proxy/friendlycodeproxy.hpp"
#include "ui/widgets/scrollbar.hpp"

client::dialogs::FriendlyCodeDialog::FriendlyCodeDialog(ui::Root& root, const String_t& title, const game::data::FriendlyCodeList_t& list, util::RequestSender<game::Session> gameSender)
    : m_root(root),
      m_title(title),
      m_gameSender(gameSender),
      m_input(3, 10, root),
      m_list(root, list)
{
    // ex WFCodeWindow::WFCodeWindow
    m_list.sig_change.add(this, &FriendlyCodeDialog::onListChange);
    m_input.sig_change.add(this, &FriendlyCodeDialog::onInputChange);
    m_input.setHotkey(util::KeyMod_Alt + 'f');
    m_input.setFont(gfx::FontRequest().addSize(1));
}

client::dialogs::FriendlyCodeDialog::~FriendlyCodeDialog()
{ }

void
client::dialogs::FriendlyCodeDialog::setFriendlyCode(const String_t& code)
{
    // ex WFCodeWindow::setFCode
    m_input.setText(code);                 // triggers onInputChange
}

String_t
client::dialogs::FriendlyCodeDialog::getFriendlyCode() const
{
    return m_input.getText();
}

bool
client::dialogs::FriendlyCodeDialog::run()
{
    // ex WFCodeWindow::init
    // VBox
    //   HBox
    //     UIStaticText "FCode:"
    //     UIInputLine
    //   HBox
    //     WFCodeListbox
    //     UIScrollbar
    //   HBox
    //     UIButton "OK"
    //     UIButton "Cancel"
    //     UIButton "Alt-R Random"
    //     UISpacer
    //     UIButton "Help"
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::Group&               g1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::FrameGroup& g2 = del.addNew(new ui::widgets::FrameGroup(ui::layout::HBox::instance0, m_root.colorScheme(), ui::widgets::FrameGroup::LoweredFrame));
    ui::Group&               g3 = del.addNew(new ui::Group(ui::layout::HBox::instance5));

    g1.add(del.addNew(new ui::widgets::StaticText(_("FCode:"), util::SkinColor::Static, gfx::FontRequest().addSize(1), m_root.provider())));
    g1.add(m_input);

    g2.add(m_list);
    g2.add(del.addNew(new ui::widgets::Scrollbar(m_list, m_root)));

    ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(_("OK"),             util::Key_Return,       m_root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(_("Cancel"),         util::Key_Escape,       m_root));
    ui::widgets::Button& btnRandom = del.addNew(new ui::widgets::Button(_("Alt-R - Random"), util::KeyMod_Alt + 'r', m_root));
    // FIXME: help
    g3.add(btnOK);
    g3.add(btnCancel);
    g3.add(btnRandom);
    g3.add(del.addNew(new ui::Spacer()));

    ui::EventLoop loop(m_root);
    btnOK.sig_fire.addNewClosure(loop.makeStop(1));
    btnCancel.sig_fire.addNewClosure(loop.makeStop(0));
    btnRandom.sig_fire.add(this, &FriendlyCodeDialog::onRandom);

    ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab));
    it.add(m_input);
    it.add(m_list);

    win.add(g1);
    win.add(g2);
    win.add(g3);
    win.add(it);
    // FIXME -> add(h.add(new WHelpWidget("pcc2:fcode")));
    // FIXME -> add(h.add(new UIQuit(cm_Escape)));
    win.pack();

    m_root.centerWidget(win);
    m_root.add(win);
    return loop.run() != 0;
}

void
client::dialogs::FriendlyCodeDialog::onListChange()
{
    // ex WFCodeWindow::scrolled
    if (m_list.hasState(ui::Widget::FocusedState)) {
        m_input.setText(m_list.getFriendlyCode());
    }
}

void
client::dialogs::FriendlyCodeDialog::onInputChange()
{
    // ex WFCodeWindow::edited
    m_list.setFriendlyCode(m_input.getText());
}

void
client::dialogs::FriendlyCodeDialog::onRandom()
{
    // ex WFCodeWindow::onRandom
    Downlink link(m_root);
    String_t result = client::proxy::FriendlyCodeProxy(m_gameSender).generateRandomCode(link);
    if (!result.empty()) {
        m_input.setText(result);
        m_input.requestFocus();
    }
}
