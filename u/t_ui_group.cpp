/**
  *  \file u/t_ui_group.cpp
  *  \brief Test for ui::Group
  */

#include "ui/group.hpp"

#include "t_ui.hpp"

/** Get that Group actually is complete (implements all virtuals). */
void
TestUiGroup::testIt()
{
    // Layout manager mock for testing
    class MyLayout : public ui::layout::Manager {
     public:
        virtual void doLayout(ui::Widget& /*container*/, gfx::Rectangle /*area*/)
            { }
        virtual ui::layout::Info getLayoutInfo(const ui::Widget& /*container*/)
            { return ui::layout::Info(); }
    };
    MyLayout layout;

    // Actual test
    ui::Group testee(layout);
}

