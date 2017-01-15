/**
  *  \file client/si/stringlistdialogwidget.cpp
  *  \brief Class client::si::StringListDialogWidget
  */

#include "client/si/stringlistdialogwidget.hpp"
#include "afl/base/deleter.hpp"
#include "ui/window.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/group.hpp"
#include "ui/eventloop.hpp"
#include "util/translation.hpp"
#include "ui/spacer.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/button.hpp"

// Constructor.
client::si::StringListDialogWidget::StringListDialogWidget(gfx::ResourceProvider& provider, ui::ColorScheme& scheme,
                                                           String_t dialogTitle, int32_t current, int32_t width, int32_t height, String_t help)
    : StringListbox(provider, scheme),
      m_dialogTitle(dialogTitle),
      m_current(current),
      m_width(width),
      m_height(height),
      m_help(help)
{ }

// Execute standard dialog.
bool
client::si::StringListDialogWidget::run(ui::Root& root)
{
    // Configure
    setPreferredHeight(m_height <= 0 ? 0 : m_height <= 3 ? 3 : m_height);
    setPreferredWidth(m_width <= 0 ? 0 : m_width, true);
    setCurrentKey(m_current);

    // We cannot operate when we have a parent
    if (getParent() != 0) {
        return false;
    }

    // ex UIListbox::doStandardListbox:
    ui::EventLoop loop(root);
    afl::base::Deleter h;

    ui::Window& w = h.addNew(new ui::Window(m_dialogTitle, root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::widgets::FrameGroup& listGroup = h.addNew(new ui::widgets::FrameGroup(ui::layout::HBox::instance0, root.colorScheme(), ui::widgets::FrameGroup::LoweredFrame));
    listGroup.add(*this);
    // FIXME: scrollbar
    // FIXME: the following assumes we get as many lines from layout as we request.
    // if (getSize() > lines)
    //     list_group.add(h.add(new UIScrollbar(*this)));
    w.add(listGroup);

    ui::Group& buttonGroup = h.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnOK     = h.addNew(new ui::widgets::Button(_("OK"), util::Key_Return, root));
    ui::widgets::Button& btnCancel = h.addNew(new ui::widgets::Button(_("Cancel"), util::Key_Escape, root));
    buttonGroup.add(btnOK);
    buttonGroup.add(btnCancel);
    buttonGroup.add(h.addNew(new ui::Spacer()));
    btnOK.sig_fire.addNewClosure(loop.makeStop(1));
    btnCancel.sig_fire.addNewClosure(loop.makeStop(0));
    // FIXME: help
    // if (!help.empty()) {
    //     button_group.add(h.add(new UIButton(_("H"), 'H')));
    //     w.add(h.add(new WHelpWidget(help)));
    // }
    w.add(buttonGroup);
    // FIXME:
    // w.add(h.add(new UIQuit(cm_Escape)));

    w.pack();
    root.centerWidget(w);
    root.add(w);
    int result = loop.run();

    // Update current
    if (result != 0) {
        getCurrentKey(m_current);
    }
    return result != 0;
}
