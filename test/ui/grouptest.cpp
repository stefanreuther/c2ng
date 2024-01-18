/**
  *  \file test/ui/grouptest.cpp
  *  \brief Test for ui::Group
  */

#include "ui/group.hpp"
#include "afl/test/testrunner.hpp"

/** Get that Group actually is complete (implements all virtuals). */
AFL_TEST_NOARG("ui.Group")
{
    // Layout manager mock for testing
    class MyLayout : public ui::layout::Manager {
     public:
        virtual void doLayout(ui::Widget& /*container*/, gfx::Rectangle /*area*/) const
            { }
        virtual ui::layout::Info getLayoutInfo(const ui::Widget& /*container*/) const
            { return ui::layout::Info(); }
    };
    MyLayout layout;

    // Actual test
    ui::Group testee(layout);
}
