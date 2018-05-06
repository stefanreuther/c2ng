/**
  *  \file ui/widgets/standarddialogbuttons.cpp
  *  \brief Class ui::widgets::StandardDialogButtons
  */

#include "ui/widgets/standarddialogbuttons.hpp"
#include "afl/base/deleter.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"

// Constructor.
ui::widgets::StandardDialogButtons::StandardDialogButtons(ui::Root& root)
    : Group(ui::layout::HBox::instance5),
      m_deleter(),
      m_root(root),
      m_pOK(0),
      m_pCancel(0)
{
    init();
}

// Destructor.
ui::widgets::StandardDialogButtons::~StandardDialogButtons()
{ }

// Attach "stop" events.
void
ui::widgets::StandardDialogButtons::addStop(EventLoop& loop)
{
    m_pOK->sig_fire.addNewClosure(loop.makeStop(1));
    m_pCancel->sig_fire.addNewClosure(loop.makeStop(0));
}

void
ui::widgets::StandardDialogButtons::init()
{
    m_pOK     = &m_deleter.addNew(new Button(_("OK"),     util::Key_Return, m_root));
    m_pCancel = &m_deleter.addNew(new Button(_("Cancel"), util::Key_Escape, m_root));
    add(*m_pOK);
    add(*m_pCancel);
    add(m_deleter.addNew(new ui::Spacer()));
}

// Execute dialog with standard dialog buttons.
bool
ui::widgets::doStandardDialog(String_t title, Widget& content, bool framed, Root& root)
{
    // Window
    afl::base::Deleter del;
    ui::Window& window = del.addNew(new ui::Window(title, root.provider(), root.colorScheme(), BLUE_WINDOW, ui::layout::VBox::instance5));

    // Content
    ui::LayoutableGroup* container = &window;
    if (framed) {
        container = &del.addNew(new FrameGroup(ui::layout::VBox::instance5, root.colorScheme(), FrameGroup::LoweredFrame));
        window.add(*container);
    }
    container->add(content);

    // Buttons
    StandardDialogButtons& buttons = del.addNew(new StandardDialogButtons(root));
    window.add(buttons);
    window.pack();

    // Operate
    EventLoop loop(root);
    buttons.addStop(loop);
    root.centerWidget(window);
    root.add(window);
    return loop.run() != 0;
}
