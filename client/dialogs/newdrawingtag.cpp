/**
  *  \file client/dialogs/newdrawingtag.cpp
  */

#include "client/dialogs/newdrawingtag.hpp"
#include "afl/base/deleter.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/group.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"
#include "ui/widgets/button.hpp"
#include "ui/spacer.hpp"
#include "client/widgets/helpwidget.hpp"

client::dialogs::NewDrawingTag::NewDrawingTag(util::StringList& tagList, ui::Root& root, util::RequestSender<game::Session> gameSender)
    : m_root(root),
      m_gameSender(gameSender),
      m_input(200, 20, root),
      m_list(root.provider(), root.colorScheme()),
      m_lastPosition(0)
{
    // Set initial text
    int32_t k;
    String_t v;
    if (tagList.get(0, k ,v)) {
        m_input.setText(v);
    }

    // Set list
    m_list.swapItems(tagList);
    m_list.setPreferredHeight(20);

    // Connect events
    m_list.sig_change.add(this, &NewDrawingTag::onMove);
    m_input.sig_change.add(this, &NewDrawingTag::onEdit);
}

client::dialogs::NewDrawingTag::~NewDrawingTag()
{ }

void
client::dialogs::NewDrawingTag::setTag(util::Atom_t atom)
{
    m_list.setCurrentKey(static_cast<int32_t>(atom));
}

void
client::dialogs::NewDrawingTag::setTagName(String_t atomName)
{
    m_input.setText(atomName);
    // triggers input's sig_change, triggers scroll
}

String_t
client::dialogs::NewDrawingTag::getTagName() const
{
    return m_input.getText();
}

bool
client::dialogs::NewDrawingTag::run(String_t title, afl::string::Translator& tx, bool* pAdjacent)
{
    // ex WCreateMarkerDialog::init
    const int CANCEL = 0, OK = 1, ADJACENT = 2;
    afl::base::Deleter del;

    // VBox
    //   HBox
    //     UIStaticText "Tag:"
    //     UIInputLine
    //   UIFrameGroup/HBox
    //     UIStandardListbox
    //     UIScrollbar
    //   HBox
    //     "OK"
    //     "Cancel"
    //     "Adj."
    //     UISpacer
    //     "Help"
    ui::Window& win = del.addNew(new ui::Window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::EventLoop loop(m_root);

    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g1.add(del.addNew(new ui::widgets::StaticText(tx("Tag:"), util::SkinColor::Static, "+", m_root.provider())));
    g1.add(m_input);
    win.add(g1);

    ui::widgets::FrameGroup& g2 = del.addNew(new ui::widgets::FrameGroup(ui::layout::VBox::instance0, m_root.colorScheme(), ui::LoweredFrame));
    g2.add(del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root)));
    win.add(g2);

    ui::Group& g3 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnHelp = del.addNew(new ui::widgets::Button(tx("Help"), 'h', m_root));
    g3.add(btnHelp);
    g3.add(del.addNew(new ui::Spacer()));

    ui::widgets::Button& btnOK = del.addNew(new ui::widgets::Button(tx("OK"), util::Key_Return, m_root));
    btnOK.sig_fire.addNewClosure(loop.makeStop(OK));
    g3.add(btnOK);

    if (pAdjacent != 0) {
        ui::widgets::Button& btnAdjacent = del.addNew(new ui::widgets::Button(tx("Adjacent"), util::KeyMod_Alt + 'a', m_root));
        btnAdjacent.sig_fire.addNewClosure(loop.makeStop(ADJACENT));
        g3.add(btnAdjacent);
    }

    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(tx("Cancel"), util::Key_Escape, m_root));
    btnCancel.sig_fire.addNewClosure(loop.makeStop(CANCEL));
    g3.add(btnCancel);
    win.add(g3);

    ui::widgets::FocusIterator& focus = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab));
    focus.add(m_input);
    focus.add(m_list);
    win.add(focus);

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, tx, m_gameSender, "pcc2:drawtag"));
    btnHelp.dispatchKeyTo(help);
    win.add(help);

    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    win.pack();

    m_root.centerWidget(win);
    m_root.add(win);

    const int result = loop.run();
    if (pAdjacent != 0) {
        *pAdjacent = (result == ADJACENT);
    }
    return result != CANCEL;
}

void
client::dialogs::NewDrawingTag::onMove()
{
    size_t newPosition = m_list.getCurrentItem();
    if (m_lastPosition != newPosition) {
        const util::StringList& list = m_list.getStringList();
        int32_t k;
        String_t v;
        if (list.get(m_list.getCurrentItem(), k, v) && v != m_input.getText()) {
            m_input.setText(v);
        }
        m_lastPosition = newPosition;
    }
}

void
client::dialogs::NewDrawingTag::onEdit()
{
    // WCreateMarkerDialog::onEdit
    String_t text = m_input.getText();

    const util::StringList& list = m_list.getStringList();
    for (size_t i = 0, n = list.size(); i != n; ++i) {
        int32_t k;
        String_t v;
        if (list.get(i, k, v) && v == text) {
            if (i != m_list.getCurrentItem()) {
                m_list.setCurrentItem(i);
            }
            break;
        }
    }
}
